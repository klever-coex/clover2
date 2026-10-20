// clover2
#include <clover2_common/util/sync_service.hpp>
#include <clover2_fcu/common/exception.hpp>
#include <clover2_fcu/mode_base.hpp>

// STL
#include <utility>

namespace clover2_fcu {

using clover2_common::util::call_sync;

mode_base::mode_base(client& fcu, const settings& s)
    : m_fcu(fcu)
    , m_settings(s) {
    if (m_settings.name.empty()) {
        throw exception("mode name must not be empty");
    }

    m_callback_group = m_fcu.callback_group();

    const std::string& bridge_node = m_fcu.base().bridge_node();
    auto& node = m_fcu.base().node();

    m_register_mode_client =
        rclcpp::create_client<clover2_fcu_msgs::srv::RegisterMode>(
            node.get_node_base_interface(), node.get_node_graph_interface(),
            node.get_node_services_interface(), bridge_node + "/register_mode",
            rclcpp::ServicesQoS(), m_callback_group);
    m_activate_mode_client =
        rclcpp::create_client<clover2_fcu_msgs::srv::ActivateMode>(
            node.get_node_base_interface(), node.get_node_graph_interface(),
            node.get_node_services_interface(), bridge_node + "/activate_mode",
            rclcpp::ServicesQoS(), m_callback_group);

    auto request =
        std::make_shared<clover2_fcu_msgs::srv::RegisterMode::Request>();
    request->name = m_settings.name;

    std::shared_ptr<clover2_fcu_msgs::srv::RegisterMode::Response> response;
    try {
        response = call_sync<clover2_fcu_msgs::srv::RegisterMode>(
            m_register_mode_client, bridge_node + "/register_mode", request,
            m_settings.wait_timeout);
    } catch (const std::runtime_error& e) {
        throw exception("mode '{}' registration failed: {}", m_settings.name,
                        e.what());
    }
    if (!response->ok) {
        throw exception("mode '{}' registration failed: {}", m_settings.name,
                        response->error);
    }
    m_mode_id = response->mode_id;

    m_bond = std::make_shared<bond::Bond>(
        "bond", std::to_string(m_mode_id), node.get_node_base_interface(),
        node.get_node_logging_interface(), node.get_node_parameters_interface(),
        node.get_node_timers_interface(), node.get_node_topics_interface(),
        [this]() { handle_mode_bond_broken(); });
    m_bond->setHeartbeatPeriod(0.10);
    m_bond->start();

    rclcpp::SubscriptionOptions options;
    options.callback_group = m_callback_group;
    m_mode_state_sub =
        rclcpp::create_subscription<clover2_fcu_msgs::msg::ModeState>(
            node, bridge_node + "/mode_state", rclcpp::QoS(1).transient_local(),
            [this](clover2_fcu_msgs::msg::ModeState::UniquePtr msg) {
                handle_mode_state(std::move(msg));
            },
            options);

    RCLCPP_INFO(m_fcu.base().logger(), "mode '%s' registered (id=%u)",
                m_settings.name.c_str(), m_mode_id);
}

mode_base::~mode_base() {
    stop_streaming();
    m_bond.reset();
}

void mode_base::completed(result r) {
    RCLCPP_INFO(m_fcu.base().logger(), "mode '%s' completed: %.*s",
                m_settings.name.c_str(), static_cast<int>(r.to_string().size()),
                r.to_string().data());

    auto request =
        std::make_shared<clover2_fcu_msgs::srv::ActivateMode::Request>();
    request->name = "";
    m_activate_mode_client->async_send_request(request);
}

void mode_base::handle_mode_state(
    clover2_fcu_msgs::msg::ModeState::UniquePtr msg) {
    const bool should_be_active =
        msg->mode_id != 0 && msg->mode_id == m_mode_id;
    if (should_be_active == m_active) {
        return;
    }

    if (should_be_active) {
        m_active = true;
        RCLCPP_INFO(m_fcu.base().logger(), "mode '%s' activated",
                    m_settings.name.c_str());
        on_activate();

        m_last_update = m_fcu.base().clock()->now();
        update_setpoint(1.0 / m_settings.update_rate_hz);
        start_update_timer();
    } else {
        stop_streaming();
        RCLCPP_INFO(m_fcu.base().logger(), "mode '%s' deactivated",
                    m_settings.name.c_str());
        on_deactivate();
    }
}

void mode_base::handle_mode_bond_broken() {
    if (!m_active) {
        return;
    }

    RCLCPP_WARN(m_fcu.base().logger(), "mode '%s' bond broken, stopping",
                m_settings.name.c_str());

    stop_streaming();
    on_deactivate();
}

void mode_base::start_update_timer() {
    auto& node = m_fcu.base().node();
    m_update_timer = rclcpp::create_timer(
        node.get_node_base_interface(), node.get_node_timers_interface(),
        m_fcu.base().clock(),
        rclcpp::Duration::from_seconds(1.0 / m_settings.update_rate_hz),
        [this]() {
            const rclcpp::Time now = m_fcu.base().clock()->now();
            const double dt = (now - m_last_update).seconds();
            m_last_update = now;
            update_setpoint(dt);
        },
        m_callback_group);
}

void mode_base::stop_update_timer() {
    if (m_update_timer) {
        m_update_timer->cancel();
        m_update_timer.reset();
    }
}

void mode_base::stop_streaming() {
    if (!m_active) {
        return;
    }

    m_active = false;
    stop_update_timer();
}

std::shared_ptr<capability::setpoint_position_interface> mode_base::position() {
    if (!m_position) {
        m_position =
            m_fcu.create_setpoint<capability::setpoint_position_interface>(
                mode_token{m_mode_id});
    }

    return m_position;
}

std::shared_ptr<capability::setpoint_velocity_interface> mode_base::velocity() {
    if (!m_velocity) {
        m_velocity =
            m_fcu.create_setpoint<capability::setpoint_velocity_interface>(
                mode_token{m_mode_id});
    }

    return m_velocity;
}

std::shared_ptr<capability::setpoint_attitude_interface> mode_base::attitude() {
    if (!m_attitude) {
        m_attitude =
            m_fcu.create_setpoint<capability::setpoint_attitude_interface>(
                mode_token{m_mode_id});
    }

    return m_attitude;
}

std::shared_ptr<capability::setpoint_rates_interface> mode_base::rates() {
    if (!m_rates) {
        m_rates = m_fcu.create_setpoint<capability::setpoint_rates_interface>(
            mode_token{m_mode_id});
    }

    return m_rates;
}

}  // namespace clover2_fcu
