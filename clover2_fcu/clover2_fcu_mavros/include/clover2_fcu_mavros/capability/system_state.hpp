#pragma once

// clover2
#include <clover2_fcu/capability/system_state_interface.hpp>
#include <clover2_fcu/context.hpp>

// ROS2
#include <rclcpp/subscription.hpp>

// msgs
#include <mavros_msgs/msg/extended_state.hpp>
#include <mavros_msgs/msg/state.hpp>

// STL
#include <string>

namespace clover2_fcu_mavros::capability {

/// /mavros/state + /mavros/extended_state telemetry. extended_state
/// landed_state feeds in_air; mavros ROS2 has no failsafe source — it
/// stays false.
class system_state : public clover2_fcu::capability::system_state_interface {
public:
    explicit system_state(const clover2_fcu::context& ctx, std::string prefix);

private:
    void publish_snapshot();

    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr m_state_sub;
    rclcpp::Subscription<mavros_msgs::msg::ExtendedState>::SharedPtr
        m_extended_sub;

    // Touched only from the fcu group (MutuallyExclusive).
    bool m_has_state{false};
    bool m_connected{false};
    bool m_armed{false};
    std::string m_mode;
    uint8_t m_landed_state{0};
};

}  // namespace clover2_fcu_mavros::capability
