// clover2
#include <clover2_fcu_mavros/capability/setpoints.hpp>

// STL
#include <utility>

namespace clover2_fcu_mavros::capability {

namespace {

using mavros_msgs::msg::PositionTarget;

constexpr uint16_t k_ignore_accel =
    PositionTarget::IGNORE_AFX | PositionTarget::IGNORE_AFY |
    PositionTarget::IGNORE_AFZ | PositionTarget::FORCE;
constexpr uint16_t k_ignore_velocity = PositionTarget::IGNORE_VX |
                                       PositionTarget::IGNORE_VY |
                                       PositionTarget::IGNORE_VZ;
constexpr uint16_t k_ignore_position = PositionTarget::IGNORE_PX |
                                       PositionTarget::IGNORE_PY |
                                       PositionTarget::IGNORE_PZ;

}  // namespace

setpoint_base::setpoint_base(const clover2_fcu::context& ctx,
                             std::string prefix)
    : m_clock(ctx.clock) {
    m_pub = ctx.publisher<mavros_msgs::msg::PositionTarget>(
        prefix + "/setpoint_raw/local", rclcpp::QoS(10));
}

void setpoint_base::publish(mavros_msgs::msg::PositionTarget target) {
    target.header.stamp = m_clock->now();
    m_pub->publish(std::move(target));
}

void position_setpoint::update(const Eigen::Vector3d& position,
                               std::optional<double> heading) {
    mavros_msgs::msg::PositionTarget target;
    target.coordinate_frame = PositionTarget::FRAME_LOCAL_NED;
    target.type_mask =
        k_ignore_accel | k_ignore_velocity | PositionTarget::IGNORE_YAW_RATE;
    if (!heading) {
        target.type_mask |= PositionTarget::IGNORE_YAW;
    }

    target.position.x = position.x();
    target.position.y = position.y();
    target.position.z = position.z();
    target.yaw = static_cast<float>(heading.value_or(0.0));

    publish(std::move(target));
}

void velocity_setpoint::update(const Eigen::Vector3d& velocity,
                               std::optional<double> yaw_rate) {
    mavros_msgs::msg::PositionTarget target;
    target.coordinate_frame = PositionTarget::FRAME_LOCAL_NED;
    target.type_mask =
        k_ignore_accel | k_ignore_position | PositionTarget::IGNORE_YAW;
    if (!yaw_rate) {
        target.type_mask |= PositionTarget::IGNORE_YAW_RATE;
    }

    target.velocity.x = velocity.x();
    target.velocity.y = velocity.y();
    target.velocity.z = velocity.z();
    target.yaw_rate = static_cast<float>(yaw_rate.value_or(0.0));

    publish(std::move(target));
}

}  // namespace clover2_fcu_mavros::capability
