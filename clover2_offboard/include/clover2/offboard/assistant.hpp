#pragma once

// clover2
#include <clover2/offboard/client.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

// STL
#include <mutex>
#include <string>

namespace clover2::offboard {

class assistant {
public:
    assistant(rclcpp::Node::SharedPtr node, client& client);

    void set_position_setpoint(double x, double y, double z, double yaw,
                              const std::string& frame_id = "map");
    void set_speed(double speed);

    void set_velocity_setpoint(double vx, double vy, double vz, double yaw_rate,
                              const std::string& frame_id = "map");

private:
    void timer_callback_50hz();

    rclcpp::Node::SharedPtr m_node;
    client* m_client;
    rclcpp::TimerBase::SharedPtr m_timer;

    std::unique_ptr<tf2_ros::Buffer> m_tf_buffer;
    std::unique_ptr<tf2_ros::TransformListener> m_tf_listener;

    std::mutex m_setpoint_mutex;
    bool m_position_mode{true};
    double m_position_x{0.0}, m_position_y{0.0}, m_position_z{0.0};
    double m_position_yaw{0.0};
    double m_velocity_x{0.0}, m_velocity_y{0.0}, m_velocity_z{0.0};
    double m_velocity_yaw_rate{0.0};
    double m_speed{1.0};
    std::string m_frame_id{"map"};
    bool m_has_setpoint{false};
};

}  // namespace clover2::offboard
