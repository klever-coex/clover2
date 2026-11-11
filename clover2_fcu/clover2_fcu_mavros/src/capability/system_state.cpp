// clover2
#include <clover2_fcu_mavros/capability/system_state.hpp>
#include <clover2_fcu_mavros/mode_mapping.hpp>

namespace clover2_fcu_mavros::capability {

namespace {

bool landed_state_in_air(uint8_t landed_state) {
    using mavros_msgs::msg::ExtendedState;
    return landed_state == ExtendedState::LANDED_STATE_IN_AIR ||
           landed_state == ExtendedState::LANDED_STATE_TAKEOFF ||
           landed_state == ExtendedState::LANDED_STATE_LANDING;
}

}  // namespace

system_state::system_state(const clover2_fcu::context& ctx, std::string prefix)
    : clover2_fcu::capability::system_state_interface(ctx.clock) {
    const auto qos = rclcpp::QoS(10).transient_local();

    m_state_sub = ctx.subscribe<mavros_msgs::msg::State>(
        prefix + "/state", qos, [this](mavros_msgs::msg::State::SharedPtr msg) {
            m_has_state = true;
            m_connected = msg->connected;
            m_armed = msg->armed;
            m_mode = mavros_mode_to_fcu(msg->mode);
            publish_snapshot();
        });

    m_extended_sub = ctx.subscribe<mavros_msgs::msg::ExtendedState>(
        prefix + "/extended_state", qos,
        [this](mavros_msgs::msg::ExtendedState::SharedPtr msg) {
            m_landed_state = msg->landed_state;
            if (m_has_state) {
                publish_snapshot();
            }
        });
}

void system_state::publish_snapshot() {
    update_data(clover2_fcu::data::system_state{
        .connected = m_connected,
        .armed = m_armed,
        .in_air = landed_state_in_air(m_landed_state),
        .failsafe = false,
        .mode = clover2_fcu::data::flight_mode{m_mode},
    });
}

}  // namespace clover2_fcu_mavros::capability
