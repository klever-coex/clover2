#include <clover2_fcu_bridge/navigation_controller.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/utils.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

#include <algorithm>
#include <cmath>

namespace {

constexpr double k_vec_eps = 1e-9;
constexpr double k_yaw_reach = 0.05;

}  // namespace

namespace clover2_fcu_bridge {

void navigation_controller::set_tolerance(double tolerance) {
    m_tolerance = tolerance;
}

void navigation_controller::set_slowdown_distance(double distance) {
    m_slowdown = std::max(distance, 1e-3);
}

void navigation_controller::set_speed_limits(double speed, double speed_limit) {
    m_speed_limit = std::max(speed_limit, 1e-3);
    m_speed = std::clamp(speed, 0.1, m_speed_limit);
}

void navigation_controller::set_yaw_rate(double yaw_rate) {
    m_yaw_rate = std::max(yaw_rate, 1e-6);
}

void navigation_controller::set_height_low(double z_min) {
    m_height_low = z_min;
}

void navigation_controller::reset() {
    m_has_target = false;
    m_target_reached = false;
    m_target = geometry_msgs::msg::PoseStamped();
    m_setpoint = geometry_msgs::msg::PoseStamped();
}

void navigation_controller::set_target(
    const geometry_msgs::msg::PoseStamped& target, double speed) {
    m_target = target;
    m_speed = std::clamp(speed, 0.1, m_speed_limit);
    m_has_target = true;
    m_target_reached = false;
}

double navigation_controller::normalize_angle(double a) {
    return std::atan2(std::sin(a), std::cos(a));
}

void navigation_controller::pose_diff(
    const geometry_msgs::msg::PoseStamped& current,
    const geometry_msgs::msg::PoseStamped& target, tf2::Vector3& diff_pos,
    double& diff_yaw) {
    tf2::Vector3 c, t;

    double cy = 0.0;
    double ty = 0.0;

    extract_pose(current, c, cy);
    extract_pose(target, t, ty);

    diff_pos = t - c;
    diff_yaw = normalize_angle(ty - cy);
}

void navigation_controller::extract_pose(
    const geometry_msgs::msg::PoseStamped& pose, tf2::Vector3& pos,
    double& yaw) {
    tf2::fromMsg(pose.pose.position, pos);
    yaw = tf2::getYaw(pose.pose.orientation);
}

void navigation_controller::set_yaw(geometry_msgs::msg::Quaternion& q,
                                    double yaw) {
    tf2::Quaternion quat;
    quat.setRPY(0.0, 0.0, yaw);
    q = tf2::toMsg(quat);
}

bool navigation_controller::pose_finite(
    const geometry_msgs::msg::PoseStamped& pose) {
    const auto& p = pose.pose.position;
    const auto& o = pose.pose.orientation;

    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
        return false;
    }

    if (!std::isfinite(o.x) || !std::isfinite(o.y) || !std::isfinite(o.z) ||
        !std::isfinite(o.w)) {
        return false;
    }

    const double n2 = o.x * o.x + o.y * o.y + o.z * o.z + o.w * o.w;
    if (!std::isfinite(n2) || n2 < 1e-6) {
        return false;
    }

    return true;
}

void navigation_controller::update(
    const geometry_msgs::msg::PoseStamped& current_pose) {
    if (!m_has_target) {
        m_setpoint = current_pose;
        m_target_reached = true;
        return;
    }

    if (!pose_finite(current_pose) || !pose_finite(m_target)) {
        m_setpoint = current_pose;
        m_target_reached = false;
        return;
    }

    tf2::Vector3 curr{};
    tf2::Vector3 tgt{};
    double curr_yaw = 0.0;
    double tgt_yaw = 0.0;
    extract_pose(current_pose, curr, curr_yaw);
    extract_pose(m_target, tgt, tgt_yaw);

    tf2::Vector3 diff = tgt - curr;
    const double dist = diff.length();
    const double diff_yaw = normalize_angle(tgt_yaw - curr_yaw);

    if (dist < m_tolerance && std::abs(diff_yaw) < k_yaw_reach) {
        m_setpoint = m_target;
        m_setpoint.header = current_pose.header;
        m_target_reached = true;
        return;
    }

    m_target_reached = false;

    tf2::Vector3 dir(0.0, 0.0, 0.0);
    if (dist > k_vec_eps) {
        dir = diff * (1.0 / dist);
    }

    const double clamp_speed =
        std::clamp(dist / m_slowdown, 0.05, 1.0) * m_speed;
    const tf2::Vector3 step = dir * clamp_speed;
    tf2::Vector3 new_pos = curr + step;

    double yaw_out = curr_yaw;
    if (std::abs(diff_yaw) <= m_yaw_rate) {
        yaw_out = tgt_yaw;
    } else {
        yaw_out = curr_yaw + std::copysign(m_yaw_rate, diff_yaw);
    }
    yaw_out = normalize_angle(yaw_out);

    m_setpoint = current_pose;
    tf2::toMsg(new_pos, m_setpoint.pose.position);
    set_yaw(m_setpoint.pose.orientation, yaw_out);

    if (m_setpoint.pose.position.z < m_height_low) {
        m_setpoint.pose.position.z = m_height_low;
    }
}

}  // namespace clover2_fcu_bridge
