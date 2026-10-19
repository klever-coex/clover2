/**
 * @file setpoint_base.cpp
 * @brief Provides the setpoint capability base implementation.
 */

// clover2
#include <clover2_fcu/capability/setpoint_base.hpp>

namespace clover2_fcu::capability {

setpoint_base::setpoint_base(base_interface& base, mode_token token)
    : m_base(base), m_logger(base.logger()), m_clock(base.clock()),
      m_mode_id(token.mode_id) {
    // Follow the bridge lease; transient local so a provider created
    // mid-flight learns the current lease immediately.
    m_mode_state_sub = rclcpp::create_subscription<
        clover2_fcu_msgs::msg::ModeState>(
        base.node(), base.bridge_node() + "/mode_state",
        rclcpp::QoS(1).transient_local(),
        [this](clover2_fcu_msgs::msg::ModeState::UniquePtr msg) {
            m_active_mode_id = msg->mode_id;
        });
}

}  // namespace clover2_fcu::capability
