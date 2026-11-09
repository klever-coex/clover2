// clover2
#include <clover2_common/util/sync_service.hpp>
#include <clover2_fcu/common/exception.hpp>
#include <clover2_fcu/mode_base.hpp>

// STL
#include <utility>

namespace clover2_fcu {

using clover2_common::util::call_sync;

mode_base::mode_base(client::shared_ptr fcu, const settings& s)
    : m_fcu(std::move(fcu))
    , m_logger(m_fcu->get_logger().get_child(s.name))
    , m_settings(s)
    , m_ctx(m_fcu->fcu_context()) {
    if (m_settings.name.empty()) {
        throw exception("mode name must not be empty");
    }

    m_register_mode_client = m_ctx.client<clover2_fcu_msgs::srv::RegisterMode>(
        m_ctx.bridge_node + "/register_mode");

    m_activate_mode_client = m_ctx.client<clover2_fcu_msgs::srv::ActivateMode>(
        m_ctx.bridge_node + "/activate_mode");

    auto request =
        std::make_shared<clover2_fcu_msgs::srv::RegisterMode::Request>();
    request->name = m_settings.name;

    std::shared_ptr<clover2_fcu_msgs::srv::RegisterMode::Response> response;
    try {
        response = call_sync<clover2_fcu_msgs::srv::RegisterMode>(
            m_register_mode_client, m_ctx.bridge_node + "/register_mode",
            request, m_settings.wait_timeout);
    } catch (const std::runtime_error& e) {
        throw exception("mode '{}' registration failed: {}", m_settings.name,
                        e.what());
    }

    if (!response->ok) {
        throw exception("mode '{}' registration failed: {}", m_settings.name,
                        response->error);
    }

    m_mode_id = response->mode_id;

    setup_bond();

    m_mode_state_sub = m_ctx.subscribe<clover2_fcu_msgs::msg::ModeState>(
        m_ctx.bridge_node + "/mode_state", rclcpp::QoS(1).transient_local(),
        [this](clover2_fcu_msgs::msg::ModeState::UniquePtr msg) {
            handle_mode_state(std::move(msg));
        });

    RCLCPP_INFO(get_logger(), "mode '%s' registered (id=%u)",
                m_settings.name.c_str(), m_mode_id);
}

void mode_base::setup_bond() {
    auto node_copy = m_ctx.node;

    // Absolute topic: a relative name would resolve against the host
    // node's namespace and never meet the bridge's bond in a non-root
    // namespace.
    m_bond = std::make_shared<bond::Bond>(
        m_ctx.bridge_node + "/bond",                //
        std::to_string(m_mode_id),                  //
        node_copy.get_node_base_interface(),        //
        node_copy.get_node_logging_interface(),     //
        node_copy.get_node_parameters_interface(),  //
        node_copy.get_node_timers_interface(),      //
        node_copy.get_node_topics_interface(),      //
        [this]() { handle_mode_bond_broken(); });
    m_bond->setHeartbeatPeriod(0.10);
    m_bond->start();
}

mode_base::~mode_base() {
    stop_streaming();

    // ~Bond synchronously invokes on_broken, which (re)creates the relay
    // timer: the timer must be torn down after the bond, not before, or a
    // [this] timer survives the object (audit: UAF on destruction).
    m_bond.reset();
    if (m_bond_relay_timer) {
        m_bond_relay_timer->cancel();
        m_bond_relay_timer.reset();
    }
}

void mode_base::completed() {
    RCLCPP_INFO(get_logger(), "mode '%s' completed", m_settings.name.c_str());

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
        RCLCPP_INFO(get_logger(), "mode '%s' activated",
                    m_settings.name.c_str());
        on_activate();
        begin_activation();
    } else {
        stop_streaming();
        RCLCPP_INFO(get_logger(), "mode '%s' deactivated",
                    m_settings.name.c_str());
        on_deactivate();
    }
}

void mode_base::handle_mode_bond_broken() {
    if (m_bond_broken.exchange(true)) {
        return;
    }

    m_bond_relay_timer = m_ctx.timer(rclcpp::Duration::from_seconds(0.001),
                                     [this]() { handle_bond_broken(); });
}

void mode_base::handle_bond_broken() {
    m_bond_relay_timer->cancel();
    m_bond_relay_timer.reset();

    if (!m_bond_broken.exchange(false) || !m_active) {
        return;
    }

    RCLCPP_WARN(get_logger(), "mode '%s' bond broken, stopping",
                m_settings.name.c_str());

    stop_streaming();
    on_deactivate();
}

void mode_base::begin_activation() {
    auto state = m_fcu->state();
    m_state_token =
        state->on_update([this](const data::system_state& snapshot) {
            handle_activation_state(snapshot);
        });

    // on_update fires on new snapshots only: consume the current one.
    if (state->has_data()) {
        handle_activation_state(state->last());
    }
}

void mode_base::handle_activation_state(const data::system_state& state) {
    if (!m_active) {
        return;
    }

    if (state.mode == data::flight_mode::offboard()) {
        m_state_token.release();
        m_last_update = m_ctx.clock->now();
        update_setpoint(1.0 / m_settings.update_rate_hz);
        start_update_timer();
        return;
    }

    if (!m_set_mode_sent.exchange(true)) {
        RCLCPP_INFO(get_logger(), "mode '%s': requesting OFFBOARD",
                    m_settings.name.c_str());
        m_fcu->commands()->set_mode(
            [client = m_activate_mode_client, logger = m_logger,
             name = m_settings.name](bool ok) {
                if (ok) {
                    return;
                }

                RCLCPP_WARN(logger,
                            "mode '%s': OFFBOARD request rejected, "
                            "surrendering the lease",
                            name.c_str());
                auto request = std::make_shared<
                    clover2_fcu_msgs::srv::ActivateMode::Request>();
                request->name = "";
                client->async_send_request(request);
            },
            data::flight_mode::offboard());
    }
}

void mode_base::start_update_timer() {
    m_update_timer = m_ctx.timer(
        rclcpp::Duration::from_seconds(1.0 / m_settings.update_rate_hz),
        [this]() {
            const rclcpp::Time now = m_ctx.clock->now();
            const double dt = (now - m_last_update).seconds();
            m_last_update = now;
            update_setpoint(dt);
        });
}

void mode_base::stop_update_timer() {
    if (m_update_timer) {
        m_update_timer->cancel();
        m_update_timer.reset();
    }
}

void mode_base::stop_streaming() {
    m_active = false;
    m_set_mode_sent = false;
    m_state_token.release();
    stop_update_timer();
}

mode_base::position_setpoint::shared_ptr mode_base::position() {
    return m_fcu->get<position_setpoint>();
}

mode_base::velocity_setpoint::shared_ptr mode_base::velocity() {
    return m_fcu->get<velocity_setpoint>();
}

}  // namespace clover2_fcu
