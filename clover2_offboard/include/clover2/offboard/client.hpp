#pragma once

// clover2
#include <clover2/offboard/bridge/base_bridge.hpp>

// Eigen
#include <Eigen/Dense>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>

namespace clover2::offboard {

class client {
public:
    explicit client(rclcpp::Node::SharedPtr node,
                   std::shared_ptr<bridge::base_bridge> bridge);

    virtual ~client() = default;

    void set_offboard_controller(bridge::data::controller_type type);
    void set_local_position_setpoint(const Eigen::Vector3d& position,
                                    double yaw);
    void set_local_velocity_setpoint(const Eigen::Vector3d& velocity,
                                    double yaw_rate);
    void set_speed(double speed);

    void arm();
    void disarm();

    Eigen::Vector3d get_local_position() const;
    Eigen::Quaterniond get_orientation() const;
    Eigen::Vector3d get_orientation_rpy() const;
    std::string get_mode() const;

private:
    rclcpp::Node::SharedPtr m_node;
    std::shared_ptr<bridge::base_bridge> m_bridge;
};

}  // namespace clover2::offboard
