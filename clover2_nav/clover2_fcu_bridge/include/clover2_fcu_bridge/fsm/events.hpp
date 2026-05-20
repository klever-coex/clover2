#pragma once

// clover2
#include <clover2_fcu_bridge/fsm/event/event_base.hpp>

// ros2
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <rclcpp/time.hpp>

namespace clover2_fcu_bridge::fsm::event {

struct event_base {
    rclcpp::Time stamp;
};

// User events
struct set_position : event_base {
    geometry_msgs::msg::PoseStamped target;
};

struct navigate : event_base {
    double nav_speed;
    double yaw_speed;
    geometry_msgs::msg::PoseStamped target;
};

struct reset : event_base {};

// Internal events
struct offboard_mode_confirmed : event_base {};
struct arm_confirmed : event_base {};
struct client_connected : event_base {};
struct client_disconnected : event_base {};

struct tick : event_base {
    geometry_msgs::msg::PoseStamped current_pose;
};

}  // namespace clover2_fcu_bridge::fsm::event
