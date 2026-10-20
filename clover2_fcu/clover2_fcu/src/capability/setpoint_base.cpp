// clover2
#include <clover2_fcu/capability/setpoint_base.hpp>

namespace clover2_fcu::capability {

setpoint_base::setpoint_base(base_interface::shared_ptr base,
                             mode_token token)
    : m_base(std::move(base))
    , m_logger(m_base->logger())
    , m_clock(m_base->clock())
    , m_mode_id(token.mode_id) {
    m_mode_state_sub =
        rclcpp::create_subscription<clover2_fcu_msgs::msg::ModeState>(
            m_base->node(), m_base->bridge_node() + "/mode_state",
            rclcpp::QoS(1).transient_local(),
            [this](clover2_fcu_msgs::msg::ModeState::UniquePtr msg) {
                m_active_mode_id.store(msg->mode_id,
                                       std::memory_order_release);
            });
}

}  // namespace clover2_fcu::capability
