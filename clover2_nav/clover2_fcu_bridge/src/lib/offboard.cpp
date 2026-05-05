// clover2
#include <clover2_fcu_bridge/backend/fabric.hpp>
#include <clover2_fcu_bridge/offboard.hpp>

// ROS2
#include <rclcpp/create_timer.hpp>
#include <rclcpp/logging.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Vector3.hpp>
#include <tf2/exceptions.h>
#include <tf2/utils.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

// STL
#include <algorithm>
#include <chrono>
#include <exception>
#include <optional>
#include <stdexcept>
#include <string>

namespace {
constexpr const double speed_low_limit = 0.1;
}

namespace clover2_fcu_bridge {

std::vector<std::string> offboard::list_backends() {
    return backend::fabric::instance().list_backends();
}

offboard::~offboard() {
    m_backend.reset();
    m_update_timer.reset();
}

bool offboard::in_idle() const {
    return m_state == state::idle;
}

void offboard::reset_state() {
    change_state(state::idle);
    m_pose_setpoint = m_backend.lock()->get_pose();

    RCLCPP_INFO(get_logger(), "State reset");
}

void offboard::set_process_callback(process_callback&& cb) {
    m_process_callback = cb;
}

void offboard::set_position(const std::string& frame_id,
                            std::optional<double> x, std::optional<double> y,
                            std::optional<double> z,
                            std::optional<double> yaw) {
    if (m_state != state::idle) {
        throw std::runtime_error("Trying set_position from invalid state.");
    }

    geometry_msgs::msg::PoseStamped pose_in_req;
    pose_in_req.header.frame_id = m_local_frame;
    complite_setpoint(frame_id, x, y, z, yaw, m_pose_setpoint, pose_in_req);

    m_pose_setpoint = pose_in_req;
    change_state(state::position);
}

void offboard::navigate(const std::string& frame_id, std::optional<double> x,
                        std::optional<double> y, std::optional<double> z,
                        std::optional<double> yaw, double speed) {
    if (m_state != state::idle) {
        throw std::runtime_error("Trying navigate from invalid state.");
    }

    m_speed = std::clamp(speed, speed_low_limit, m_speed_limit);
    geometry_msgs::msg::PoseStamped pose_in_req;
    pose_in_req.header.frame_id = m_local_frame;
    complite_setpoint(frame_id, x, y, z, yaw, m_pose_setpoint, pose_in_req);

    // TODO: update m_yaw_speed

    m_nav_setpoint = pose_in_req;
    change_state(state::navigation);
}

void offboard::get_position(double& x, double& y, double& z,
                            double& yaw) const {
    const auto pose = m_backend.lock()->get_pose();
    extract_pose(pose, x, y, z, yaw);
}

geometry_msgs::msg::PoseStamped offboard::get_position() const {
    return m_backend.lock()->get_pose();
}

void offboard::extract_pose(const geometry_msgs::msg::PoseStamped& pose,
                            double& x, double& y, double& z,
                            double& yaw) const {
    x = pose.pose.position.x;
    y = pose.pose.position.y;
    z = pose.pose.position.z;
    yaw = tf2::getYaw(pose.pose.orientation);
}

void offboard::extract_pose(const geometry_msgs::msg::PoseStamped& pose,
                            tf2::Vector3& pos, double& yaw) const {
    tf2::fromMsg(pose.pose.position, pos);
    yaw = tf2::getYaw(pose.pose.orientation);
}

void offboard::complite_setpoint(
    const std::string& frame_id, const std::optional<double>& x,
    const std::optional<double>& y, const std::optional<double>& z,
    const std::optional<double>& yaw,
    const geometry_msgs::msg::PoseStamped& ref_pose,
    geometry_msgs::msg::PoseStamped& pose) const {
    geometry_msgs::msg::PoseStamped pose_in_req = ref_pose;
    pose_in_req.header.stamp = get_clock()->now();

    try {
        if (!ref_pose.header.frame_id.empty()) {
            if (ref_pose.header.frame_id != frame_id) {
                pose_in_req = m_tf_buffer->transform(ref_pose, frame_id,
                                                     tf2::durationFromSec(0.1));
            } else {
                pose_in_req.header.frame_id = frame_id;
            }
        } else {
            pose_in_req.header.frame_id = frame_id;
        }
    } catch (const tf2::TransformException& ex) {
        RCLCPP_WARN(get_logger(),
                    "complite_setpoint: TF to frame_id '%s' failed: %s",
                    frame_id.c_str(), ex.what());
        return;
    }

    const auto& prev_pos = pose_in_req.pose.position;
    pose_in_req.pose.position.x = x.value_or(prev_pos.x);
    pose_in_req.pose.position.y = y.value_or(prev_pos.y);
    pose_in_req.pose.position.z = z.value_or(prev_pos.z);

    if (yaw.has_value()) {
        set_yaw(pose_in_req.pose.orientation, *yaw);
    }

    pose_in_req.header.frame_id = frame_id;
    pose_in_req.header.stamp = get_clock()->now();

    try {
        if (frame_id != m_local_frame) {
            pose = m_tf_buffer->transform(pose_in_req, m_local_frame,
                                          tf2::durationFromSec(0.1));
        } else {
            pose = pose_in_req;
        }
    } catch (const tf2::TransformException& ex) {
        RCLCPP_WARN(
            get_logger(),
            "complite_setpoint: TF to local frame '%s' failed: %s (setpoint "
            "unchanged)",
            m_local_frame.c_str(), ex.what());
        return;
    }

    if (pose.pose.position.z < m_height_low) {
        RCLCPP_WARN(get_logger(), "Setpoint to low %.03fm. Clamp to %.03fm",
                    pose.pose.position.z, m_height_low);
        pose.pose.position.z = m_height_low;
    }
}

void offboard::compute_diff(const geometry_msgs::msg::PoseStamped& ref,
                            tf2::Vector3& diff_pos, double& diff_yaw) const {
    tf2::Vector3 curr_pos, target_pos;
    double curr_yaw, target_yaw;

    extract_pose(m_current_pose, curr_pos, curr_yaw);
    extract_pose(ref, target_pos, target_yaw);

    diff_pos = target_pos - curr_pos;
    diff_yaw = target_yaw - curr_yaw;
}

void offboard::nav_current_diff(tf2::Vector3& diff_pos, double& diff_yaw) const {
    compute_diff(m_nav_setpoint, diff_pos, diff_yaw);
}

void offboard::set_yaw(geometry_msgs::msg::Quaternion& q, double yaw) const {
    tf2::Quaternion quat;
    quat.setRPY(0.0, 0.0, yaw);
    q = tf2::toMsg(quat);
}

void offboard::check_fcu(state& process_state) {
    auto backend = m_backend.lock();
    const auto mode = backend->get_mode();

    if (mode != data::mode::value::offboard) {
        try {
            backend->set_mode(data::mode::value::offboard);
            m_reset_require = true;
        } catch (const std::exception& e) {
            RCLCPP_ERROR(get_logger(), "Fail to set offboard mode: %s",
                         e.what());
            throw;
        }

        process_state = state::idle;
        return;
    }

    if (!backend->is_armed()) {
        backend->arm();

        m_reset_require = true;
        process_state = state::idle;
        return;
    }

    m_check_fcu = false;
}

void offboard::publish_offboard() {
    auto process_state = m_state;
    auto backend = m_backend.lock();

    if (!backend->ready()) return;

    m_current_pose = backend->get_pose();
    m_pose_setpoint.header.stamp = get_clock()->now();
    geometry_msgs::msg::PoseStamped target_pose;

    if (m_state != state::idle && m_check_fcu) {
        check_fcu(process_state);
    }

    if (m_reset_require) {
        RCLCPP_INFO(get_logger(), "Pose reset require");
        m_pose_setpoint = m_current_pose;
        m_reset_require = false;
    }

    switch (process_state) {
        case state::idle:
            publish_position();
            break;
        case state::position:
            publish_position();
            target_pose = m_pose_setpoint;
            break;
        case state::navigation:
            update_navigation_setpoint();
            publish_position();
            target_pose = m_nav_setpoint;
            break;
        default:
            RCLCPP_ERROR(get_logger(), "Unknown state %d", m_state);
            break;
    }

    if (m_state != state::idle) {
        check_action_complite(target_pose);
    }

    if (m_process_callback) {
        m_process_callback();
    }
}

void offboard::publish_position() {
    double x, y, z, yaw;
    extract_pose(m_pose_setpoint, x, y, z, yaw);
    m_backend.lock()->set_position_setpoint(x, y, z, yaw);
}

void offboard::publish_velocity() {
    double vx, vy, vz, yaw_rate;
    vx = m_velocity_setpoint.linear.x;
    vy = m_velocity_setpoint.linear.y;
    vz = m_velocity_setpoint.linear.z;
    yaw_rate = m_velocity_setpoint.angular.z;
    m_backend.lock()->set_velocity_setpoint(vx, vy, vz, yaw_rate);
}

void offboard::update_navigation_setpoint() {
    tf2::Vector3 curr_pos, nav_pos;
    double curr_yaw, nav_yaw;

    extract_pose(m_current_pose, curr_pos, curr_yaw);
    extract_pose(m_nav_setpoint, nav_pos, nav_yaw);

    auto diff_pos = nav_pos - curr_pos;
    auto diff_yaw = nav_yaw - curr_yaw;

    double clamp_speed =
        std::clamp(diff_pos.length() / m_slowdown_distance, 0.1, 1.0) * m_speed;

    nav_pos = diff_pos.normalized() * clamp_speed;
    nav_pos += curr_pos;

    double yaw_pose = curr_yaw + m_yaw_speed * (std::abs(diff_yaw) / diff_yaw);
    if (std::abs(diff_yaw) < m_yaw_speed) {
        yaw_pose = nav_yaw;
    }

    tf2::toMsg(nav_pos, m_pose_setpoint.pose.position);

    set_yaw(m_pose_setpoint.pose.orientation, yaw_pose);
}

void offboard::check_action_complite(
    const geometry_msgs::msg::PoseStamped& target_pose) {
    double diff_yaw;
    tf2::Vector3 diff_pos;
    compute_diff(target_pose, diff_pos, diff_yaw);

    if (diff_pos.length() < m_tolerance && std::abs(diff_yaw) < 0.05) {
        change_state(state::idle);
        m_pose_setpoint = target_pose;
    }
}

void offboard::change_state(const state new_state) {
    auto state_set_map = [](const offboard::state& s) -> const char* {
        switch (s) {
            case offboard::state::idle:
                return "idle";
            case offboard::state::position:
                return "position";
            case offboard::state::navigation:
                return "navigation";
            default:
                throw std::runtime_error("Unexpected state");
        }
    };

    RCLCPP_INFO(get_logger(), "Change state from %s to %s",
                state_set_map(m_state), state_set_map(new_state));
    m_state = new_state;

    if (new_state != offboard::state::idle) {
        m_check_fcu = true;
    }
}

}  // namespace clover2_fcu_bridge
