#pragma once

// clover2
#include <clover2_fcu/capability/odometry_interface.hpp>
#include <clover2_fcu/context.hpp>

// ROS2
#include <rclcpp/subscription.hpp>

// msgs
#include <nav_msgs/msg/odometry.hpp>

namespace clover2_fcu_mavros::capability {

/// /mavros/local_position/odom telemetry.
class odometry : public clover2_fcu::capability::odometry_interface {
public:
    explicit odometry(const clover2_fcu::context& ctx, std::string prefix);

private:
    rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr m_odom_sub;
};

}  // namespace clover2_fcu_mavros::capability
