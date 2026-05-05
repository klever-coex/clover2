#pragma once

// clover2
#include <clover2_fcu_bridge/backend/base_backend.hpp>
#include <clover2_fcu_bridge/backend/context.hpp>

// ROS2
#include <mavros_msgs/msg/position_target.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/command_tol.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <rclcpp/publisher.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp/subscription.hpp>

namespace clover2_fcu_bridge::backend {

class mavros : public base_backend {
public:
    static constexpr const char* name = "mavros";

    explicit mavros(const context& ctx);
    ~mavros() override = default;

    bool is_armed() const final { return m_mavros_state.armed; };
    bool ready() const final;
    void arm() final;
    void disarm() final;
    void land() final;

    void set_mode(const data::mode& mode) final;
    data::mode get_mode() const final;

    void set_setpoint(const std::optional<tf2::Vector3> p,
                      const std::optional<tf2::Vector3> v,
                      const std::optional<tf2::Vector3> a,
                      const std::optional<double> yaw,
                      const std::optional<double> yaw_rate) final;

private:
    rclcpp::Publisher<mavros_msgs::msg::PositionTarget>::SharedPtr
        m_pos_setpoint_pub;

    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr m_pose_sub;
    rclcpp::Subscription<mavros_msgs::msg::State>::SharedPtr m_state_sub;

    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr m_arming_client;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr m_set_mode_client;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr m_land_client;

    data::mode m_mode;
    mavros_msgs::msg::State m_mavros_state;
    bool m_pose_received{false};
};

}  // namespace clover2_fcu_bridge::backend
