#pragma once

// clover2
#include <clover2_fcu/capability/setpoint_position_interface.hpp>
#include <clover2_fcu/capability/setpoint_velocity_interface.hpp>
#include <clover2_fcu/context.hpp>

// ROS2
#include <rclcpp/publisher.hpp>

// msgs
#include <mavros_msgs/msg/position_target.hpp>

// STL
#include <string>

namespace clover2_fcu_mavros::capability {

/// Shared publishing machinery for setpoint_raw/local (PositionTarget,
/// FRAME_LOCAL_NED — mavros converts ENU input to the FCU NED frame).
class setpoint_base {
public:
    explicit setpoint_base(const clover2_fcu::context& ctx, std::string prefix);

protected:
    void publish(mavros_msgs::msg::PositionTarget target);

    rclcpp::Publisher<mavros_msgs::msg::PositionTarget>::SharedPtr m_pub;
    rclcpp::Clock::SharedPtr m_clock;
};

/// Position setpoint; heading is optional (yaw ignored when absent).
class position_setpoint
    : public clover2_fcu::capability::setpoint_position_interface,
      private setpoint_base {
public:
    explicit position_setpoint(const clover2_fcu::context& ctx,
                               std::string prefix)
        : setpoint_base(ctx, std::move(prefix)) {}

    void update(const Eigen::Vector3d& position,
                std::optional<double> heading) override;
};

/// Velocity setpoint; yaw_rate is optional.
class velocity_setpoint
    : public clover2_fcu::capability::setpoint_velocity_interface,
      private setpoint_base {
public:
    explicit velocity_setpoint(const clover2_fcu::context& ctx,
                               std::string prefix)
        : setpoint_base(ctx, std::move(prefix)) {}

    void update(const Eigen::Vector3d& velocity,
                std::optional<double> yaw_rate) override;
};

}  // namespace clover2_fcu_mavros::capability
