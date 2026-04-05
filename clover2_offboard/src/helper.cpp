// clover2
#include "clover2_offboard/data/mode.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <clover2_offboard/backend/fabric.hpp>
#include <clover2_offboard/helper.hpp>

// ROS2
#include <rclcpp/create_timer.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Vector3.hpp>
#include <tf2/exceptions.h>
#include <tf2/utils.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

// STL
#include <algorithm>
#include <chrono>
#include <exception>
#include <stdexcept>
#include <string>

namespace clover2_offboard {

std::vector<std::string> helper::list_backends() {
    return backend::fabric::instance().list_backends();
}

helper::~helper() {
    m_backend.reset();
    m_update_timer.reset();
}

void helper::set_position(const std::string& frame_id, std::optional<double> x,
                          std::optional<double> y, std::optional<double> z,
                          std::optional<double> yaw) {
    if (m_state != state::none) {
        throw std::runtime_error("Trying set_position from invalid state.");
    }

    geometry_msgs::msg::PoseStamped pose_in_req;
    complite_setpoint(frame_id, x, y, z, yaw, m_pose_setpoint, pose_in_req);

    m_pose_setpoint = pose_in_req;
    change_state(state::position);
}

void helper::navigate(const std::string& frame_id, std::optional<double> x,
                      std::optional<double> y, std::optional<double> z,
                      std::optional<double> yaw, double speed) {
    if (m_state != state::none) {
        throw std::runtime_error("Trying navigate from invalid state.");
    }

    m_speed = speed;
    geometry_msgs::msg::PoseStamped pose_in_req;
    complite_setpoint(frame_id, x, y, z, yaw, m_pose_setpoint, pose_in_req);

    m_nav_setpoint = pose_in_req;
    change_state(state::navigation);
}

void helper::get_position(double& x, double& y, double& z, double& yaw) const {
    const auto pose = m_backend->get_pose();
    extract_pose(pose, x, y, z, yaw);
}

void helper::extract_pose(const geometry_msgs::msg::PoseStamped& pose,
                          double& x, double& y, double& z, double& yaw) const {
    x = pose.pose.position.x;
    y = pose.pose.position.y;
    z = pose.pose.position.z;
    yaw = tf2::getYaw(pose.pose.orientation);
}

void helper::extract_pose(const geometry_msgs::msg::PoseStamped& pose,
                          tf2::Vector3& pos, double& yaw) {
    tf2::fromMsg(pose.pose.position, pos);
    yaw = tf2::getYaw(pose.pose.orientation);
}

void helper::complite_setpoint(const std::string& frame_id,
                               const std::optional<double>& x,
                               const std::optional<double>& y,
                               const std::optional<double>& z,
                               const std::optional<double>& yaw,
                               const geometry_msgs::msg::PoseStamped& ref_pose,
                               geometry_msgs::msg::PoseStamped& pose) const {
    geometry_msgs::msg::PoseStamped pose_in_req = ref_pose;
    pose_in_req.header.stamp = get_clock()->now();

    try {
        if (!pose.header.frame_id.empty()) {
            if (pose.header.frame_id != frame_id) {
                pose_in_req = m_tf_buffer->transform(pose, frame_id);
            } else {
                pose_in_req.header.frame_id = frame_id;
            }
        } else {
            pose_in_req.header.frame_id = frame_id;
        }
    } catch (const tf2::TransformException& ex) {
        RCLCPP_WARN(get_logger(),
                    "set_position: TF to frame_id '%s' failed: %s",
                    frame_id.c_str(), ex.what());
        return;
    }

    const auto& prev_pos = pose_in_req.pose.position;
    pose_in_req.pose.position.x = x.value_or(prev_pos.x);
    pose_in_req.pose.position.y = y.value_or(prev_pos.y);
    pose_in_req.pose.position.z = z.value_or(prev_pos.z);

    if (yaw.has_value()) {
        tf2::Quaternion q;
        q.setRPY(0.0, 0.0, *yaw);
        pose_in_req.pose.orientation.x = q.x();
        pose_in_req.pose.orientation.y = q.y();
        pose_in_req.pose.orientation.z = q.z();
        pose_in_req.pose.orientation.w = q.w();
    }

    pose_in_req.header.frame_id = frame_id;
    pose_in_req.header.stamp = get_clock()->now();

    try {
        if (frame_id != m_local_frame) {
            pose = m_tf_buffer->transform(pose_in_req, m_local_frame);
        } else {
            pose = pose_in_req;
        }
    } catch (const tf2::TransformException& ex) {
        RCLCPP_WARN(get_logger(),
                    "set_position: TF to local frame '%s' failed: %s (setpoint "
                    "unchanged)",
                    m_local_frame.c_str(), ex.what());
        return;
    }
}

void helper::check_fcu(state& process_state) {
    const auto mode = m_backend->get_mode();

    if (mode != data::mode::value::offboard) {
        try {
            m_backend->set_mode(data::mode::value::offboard);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(get_logger(), "Fail to set offboard mode: %s",
                         e.what());
            throw;
        }

        process_state = state::none;
    }
}

void helper::publish_offboard() {
    auto process_state = m_state;

    if (m_state != state::none) {
        check_fcu(process_state);
    }

    switch (process_state) {
        case state::none:
            m_velocity_setpoint = geometry_msgs::msg::Twist();
            publish_position();
            break;
        case state::position:
            publish_position();
            break;
        case state::navigation:
            update_navigation_setpoint();
            publish_position();
            break;
        default:
            RCLCPP_ERROR(get_logger(), "Unknown state %d", m_state);
            break;
    }

    if (m_state != state::none) {
        check_action_complite();
    }
}

void helper::publish_position() {
    double x, y, z, yaw;
    extract_pose(m_pose_setpoint, x, y, z, yaw);
    m_backend->set_position_setpoint(x, y, z, yaw);
}

void helper::publish_velocity() {
    double vx, vy, vz, yaw_rate;
    vx = m_velocity_setpoint.linear.x;
    vy = m_velocity_setpoint.linear.y;
    vz = m_velocity_setpoint.linear.z;
    yaw_rate = m_velocity_setpoint.angular.z;
    m_backend->set_velocity_setpoint(vx, vy, vz, yaw_rate);
}

void helper::update_navigation_setpoint() {
    const auto current_pose = m_backend->get_pose();

    tf2::Vector3 curr_pos, nav_pos;
    double curr_yaw, nav_yaw;

    extract_pose(current_pose, curr_pos, curr_yaw);
    extract_pose(m_nav_setpoint, nav_pos, nav_yaw);

    auto diff_pos = nav_pos - curr_pos;
    auto diff_yaw = nav_yaw - curr_yaw;

    double clamp_speed =
        std::clamp(diff_pos.length() / m_slowdown_distance, 0.1, 1.0) * m_speed;

    double remain_time = diff_pos.length() / clamp_speed;
    auto dt = std::chrono::duration<double>(m_publish_period).count();

    double yaw_pose = (diff_yaw / remain_time) * dt;

    nav_pos = diff_pos.normalized() * clamp_speed;
    nav_pos += curr_pos;

    tf2::toMsg(nav_pos, m_pose_setpoint.pose.position);

    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, yaw_pose);
    m_pose_setpoint.pose.orientation.x = q.x();
    m_pose_setpoint.pose.orientation.y = q.y();
    m_pose_setpoint.pose.orientation.z = q.z();
    m_pose_setpoint.pose.orientation.w = q.w();
}

void helper::check_action_complite() {
    const auto current_pose = m_backend->get_pose();

    tf2::Vector3 curr_pos, nav_pos;
    double curr_yaw, nav_yaw;

    extract_pose(current_pose, curr_pos, curr_yaw);
    extract_pose(m_nav_setpoint, nav_pos, nav_yaw);

    auto diff_pos = nav_pos - curr_pos;
    auto diff_yaw = nav_yaw - curr_yaw;

    if (diff_pos.length() < m_tolerance && std::abs(diff_yaw) < 0.1) {
        change_state(state::none);
        m_pose_setpoint = m_nav_setpoint;
    }
}

void helper::change_state(const state new_state) {
    auto state_set_map = [](const helper::state& s) -> const char* {
        switch (s) {
            case clover2_offboard::helper::state::none:
                return "none";
            case clover2_offboard::helper::state::position:
                return "position";
            case clover2_offboard::helper::state::navigation:
                return "navigation";
            default:
                throw std::runtime_error("Unexpected state");
        }
    };

    RCLCPP_INFO(get_logger(), "Change state from %s to %s",
                state_set_map(m_state), state_set_map(new_state));
    m_state = new_state;
}

}  // namespace clover2_offboard
