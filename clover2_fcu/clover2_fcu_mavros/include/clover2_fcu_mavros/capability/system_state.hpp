#pragma once

// clover2
#include <clover2_fcu/capability/system_state_interface.hpp>
#include <clover2_fcu_mavros/capability/base.hpp>

// ROS2
#include <rclcpp/subscription.hpp>

// msgs
#include <mavros_msgs/msg/state.hpp>

namespace clover2_fcu_mavros::capability {

class system_state : public clover2_fcu::capability::system_state_interface {
public:
    explicit system_state(base::shared_ptr base);

private:
    base::shared_ptr m_base;

    rclcpp::Subscription<mavros_msgs::msg::State> m_state_sub;
};

}  // namespace clover2_fcu_mavros::capability
