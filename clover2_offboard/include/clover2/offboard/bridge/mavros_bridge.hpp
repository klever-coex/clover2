#pragma once

// clover2
#include <clover2/offboard/bridge/base_bridge.hpp>
#include <clover2/offboard/bridge/creation_context.hpp>

// Eigen
#include <Eigen/Dense>

// ROS2
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <mavros_msgs/msg/position_target.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <mutex>
#include <optional>

namespace clover2::offboard::bridge {

class mavros_bridge : public base_bridge {
public:
    static constexpr const char* name = "mavros";

    explicit mavros_bridge(const creation_context& ctx);

    std::string get_mode() const override;

    void set_local_position_setpoint(const Eigen::Vector3d& position,
                                    double yaw) override;
    void set_local_velocity_setpoint(const Eigen::Vector3d& velocity,
                                    double yaw_rate) override;
    void set_mode(const std::string& mode) override;
    void set_offboard_controller(data::controller_type type) override;
    void set_speed(double speed) override;

    void arm() override;
    void disarm() override;

private:
    rclcpp::Publisher<mavros_msgs::msg::PositionTarget>::SharedPtr m_setpoint_pub;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr m_set_mode_client;
    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr m_arming_client;

    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr m_state_sub;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr m_pose_sub;
    rclcpp::Subscription<geometry_msgs::msg::TwistStamped>::SharedPtr m_velocity_sub;

    mutable std::mutex m_state_mutex;
    std::optional<mavros_msgs::msg::State> m_state;
    std::optional<geometry_msgs::msg::PoseStamped> m_pose;
    std::optional<geometry_msgs::msg::TwistStamped> m_velocity;

    double m_speed_limit{1.0};

    void state_callback(const mavros_msgs::msg::State::SharedPtr msg);
    void pose_callback(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
    void velocity_callback(const geometry_msgs::msg::TwistStamped::SharedPtr msg);
};

}  // namespace clover2::offboard::bridge
