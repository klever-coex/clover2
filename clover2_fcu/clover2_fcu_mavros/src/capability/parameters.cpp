// clover2
#include <clover2_fcu_mavros/capability/parameters.hpp>

// clover2
#include <clover2_common/util/sync_service.hpp>

// STL
#include <stdexcept>
#include <utility>

namespace clover2_fcu_mavros::capability {

using clover2_fcu::result;

parameters::parameters(const clover2_fcu::context& ctx, std::string prefix)
    : m_logger(ctx.logger) {
    m_timer_factory = [this, ctx](rclcpp::Duration period) {
        return ctx.timer(period, [this] { attempt_pull(); });
    };

    auto node_copy = ctx.node;
    m_params_group = node_copy.get_node_base_interface()->create_callback_group(
        rclcpp::CallbackGroupType::MutuallyExclusive);

    m_pull_client = ctx.client<mavros_msgs::srv::ParamPull>(
        prefix + "/param/pull", rclcpp::ServicesQoS(), m_params_group);
    m_set_client = ctx.client<mavros_msgs::srv::ParamSetV2>(
        prefix + "/param/set", rclcpp::ServicesQoS(), m_params_group);

    m_event_sub = ctx.subscribe<mavros_msgs::msg::ParamEvent>(
        prefix + "/param/event", rclcpp::ParameterEventsQoS(),
        [this](mavros_msgs::msg::ParamEvent::SharedPtr msg) {
            handle_event(*msg);
        });

    attempt_pull();
}

void parameters::attempt_pull() {
    if (!m_pull_client->service_is_ready()) {
        if (!m_retry_timer) {
            m_retry_timer = m_timer_factory(rclcpp::Duration::from_seconds(2));
        }

        return;
    }

    m_retry_timer.reset();

    auto request = std::make_shared<mavros_msgs::srv::ParamPull::Request>();
    request->force_pull = false;
    std::weak_ptr<parameters> self = weak_from_this();
    m_pull_client->async_send_request(
        request,
        [self = std::move(self)](
            rclcpp::Client<mavros_msgs::srv::ParamPull>::SharedFuture future) {
            const auto& response = *future.get();
            const auto provider = self.lock();
            if (!provider) {
                return;
            }

            const std::scoped_lock lock(provider->m_mutex);
            provider->m_pulled = response.success;
            if (!response.success) {
                RCLCPP_WARN(provider->m_logger,
                            "parameters: pull did not complete");
            }
        });
}

void parameters::handle_event(const mavros_msgs::msg::ParamEvent& msg) {
    rclcpp::ParameterValue value(msg.value);
    store(msg.param_id, std::move(value));
}

std::optional<rclcpp::ParameterValue> parameters::cached(
    const std::string& name) const {
    const std::scoped_lock lock(m_mutex);
    const auto it = m_cache.find(name);
    if (it == m_cache.end()) {
        return std::nullopt;
    }

    return it->second;
}

void parameters::store(const std::string& name, rclcpp::ParameterValue value) {
    const std::scoped_lock lock(m_mutex);
    m_cache.insert_or_assign(name, std::move(value));
}

result parameters::get(const std::string& name, rclcpp::ParameterValue& out) {
    if (const auto value = cached(name)) {
        out = std::move(*value);
        return result::value::success;
    }

    return m_pulled ? result::value::rejected : result::value::timeout;
}

result parameters::set(const std::string& name,
                       const rclcpp::ParameterValue& value) {
    auto request = std::make_shared<mavros_msgs::srv::ParamSetV2::Request>();
    request->param_id = name;
    request->value = value.to_value_msg();
    request->force_set = false;

    try {
        const auto response =
            clover2_common::util::call_sync<mavros_msgs::srv::ParamSetV2>(
                m_set_client, m_set_client->get_service_name(), request,
                m_call_timeout);
        if (!response->success) {
            return result::value::rejected;
        }
    } catch (const std::runtime_error& e) {
        RCLCPP_WARN(m_logger, "parameters: set '%s' failed: %s", name.c_str(),
                    e.what());
        return result::value::timeout;
    }

    store(name, value);
    return result::value::success;
}

std::vector<std::string> parameters::list() const {
    std::vector<std::string> result;
    result.reserve(m_cache.size());
    for (const auto& [name, _] : m_cache) {
        result.push_back(name);
    }

    return result;
}

}  // namespace clover2_fcu_mavros::capability
