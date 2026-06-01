// clover2
#include <clover2_fcu_bridge/backend/fabric.hpp>
#include <clover2_fcu_bridge/offboard.hpp>

// ROS2
#include <rclcpp/create_timer.hpp>
#include <rclcpp/logging.hpp>
#include <tf2/LinearMath/Quaternion.h>
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
constexpr const double k_speed_low_limit = 0.1;
}  // namespace

namespace clover2_fcu_bridge {

std::vector<std::string> offboard::list_backends() {
    return backend::fabric::instance().list_backends();
}

offboard::~offboard() { m_update_timer.reset(); }

bool offboard::in_idle() const { return m_fsm.quiescent_for_async(); }

bool offboard::in_error() const {
    return m_fsm.current_state() == offboard_fsm::state::error;
}

void offboard::reset_state() {
    m_nav.reset();
    m_fsm.reset();
    try {
        m_pose_setpoint = m_fcu.get_pose();
    } catch (const std::exception&) {
        m_pose_setpoint = geometry_msgs::msg::PoseStamped();
    }
    m_fcu.set_mode(data::mode{data::mode::value::position});
    RCLCPP_INFO(get_logger(), "State reset");
}

void offboard::set_process_callback(process_callback&& cb) {
    m_process_callback = std::move(cb);
}

void offboard::set_speed_limit(double speed) {  //
    m_nav.set_speed_limit(speed);
}

double offboard::get_speed_limit() const {  //
    return m_nav.get_speed_limit();
}

void offboard::set_tolerance(double tolerance) {
    m_nav.set_tolerance(tolerance);
}

double offboard::get_tolerance() const {  //
    return m_nav.get_tolerance();
}

void offboard::set_slowdown_distance(double distance) {
    m_nav.set_slowdown_distance(distance);
}

double offboard::get_slowdown_distance() const {
    return m_nav.get_slowdown_distance();
}

void offboard::set_position(const std::string& frame_id,
                            std::optional<double> x, std::optional<double> y,
                            std::optional<double> z,
                            std::optional<double> yaw) {
    if (!m_fsm.can_accept_command()) {
        throw std::runtime_error("Trying set_position from invalid state.");
    }

    geometry_msgs::msg::PoseStamped pose_in_req;
    pose_in_req.header.frame_id = m_local_frame;
    try {
        complete_setpoint(frame_id, x, y, z, yaw, m_pose_setpoint, pose_in_req);
    } catch (const tf2::TransformException& ex) {
        on_tf_failed(ex.what());
        throw;
    }

    m_fsm.request_position(pose_in_req, m_fcu, m_nav, m_clock->now());
}

void offboard::navigate(const std::string& frame_id, std::optional<double> x,
                        std::optional<double> y, std::optional<double> z,
                        std::optional<double> yaw, double speed) {
    if (!m_fsm.can_accept_command()) {
        throw std::runtime_error("Trying navigate from invalid state.");
    }

    const double clamped = std::clamp(speed, k_speed_low_limit, m_nav.get_speed_limit());
    m_nav.set_speed(clamped);
    m_nav.set_yaw_rate(m_nav.get_yaw_rate());

    geometry_msgs::msg::PoseStamped pose_in_req;
    pose_in_req.header.frame_id = m_local_frame;
    try {
        complete_setpoint(frame_id, x, y, z, yaw, m_pose_setpoint, pose_in_req);
    } catch (const tf2::TransformException& ex) {
        on_tf_failed(ex.what());
        throw;
    }

    m_fsm.request_navigate(pose_in_req, clamped, m_fcu, m_nav, m_clock->now());
}

void offboard::get_position(double& x, double& y, double& z,
                            double& yaw) const {
    const auto pose = m_fcu.get_pose();
    extract_pose(pose, x, y, z, yaw);
}

geometry_msgs::msg::PoseStamped offboard::get_position() const {
    return m_fcu.get_pose();
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

void offboard::complete_setpoint(
    const std::string& frame_id, const std::optional<double>& x,
    const std::optional<double>& y, const std::optional<double>& z,
    const std::optional<double>& yaw,
    const geometry_msgs::msg::PoseStamped& ref_pose,
    geometry_msgs::msg::PoseStamped& pose) const {
    geometry_msgs::msg::PoseStamped pose_in_req = ref_pose;

    try {
        if (!ref_pose.header.frame_id.empty()) {
            if (ref_pose.header.frame_id != frame_id) {
                geometry_msgs::msg::PoseStamped ref_tf = ref_pose;
                ref_tf.header.stamp.sec = 0;
                ref_tf.header.stamp.nanosec = 0;
                pose_in_req = m_tf_buffer->transform(
                    ref_tf, frame_id, tf2::durationFromSec(0.05));
            } else {
                pose_in_req.header.frame_id = frame_id;
            }
        } else {
            pose_in_req.header.frame_id = frame_id;
        }
    } catch (const tf2::TransformException& ex) {
        RCLCPP_WARN(get_logger(),
                    "complete_setpoint: TF to frame_id '%s' failed: %s",
                    frame_id.c_str(), ex.what());
        throw;
    }

    const auto& prev_pos = pose_in_req.pose.position;
    pose_in_req.pose.position.x = x.value_or(prev_pos.x);
    pose_in_req.pose.position.y = y.value_or(prev_pos.y);
    pose_in_req.pose.position.z = z.value_or(prev_pos.z);

    if (yaw.has_value()) {
        set_yaw(pose_in_req.pose.orientation, *yaw);
    }

    pose_in_req.header.frame_id = frame_id;
    pose_in_req.header.stamp.sec = 0;
    pose_in_req.header.stamp.nanosec = 0;

    try {
        if (frame_id != m_local_frame) {
            pose = m_tf_buffer->transform(pose_in_req, m_local_frame,
                                          tf2::durationFromSec(0.05));
        } else {
            pose = pose_in_req;
        }
    } catch (const tf2::TransformException& ex) {
        RCLCPP_WARN(get_logger(),
                    "complete_setpoint: TF to local frame '%s' failed: %s",
                    m_local_frame.c_str(), ex.what());
        throw;
    }

    if (pose.header.frame_id != m_local_frame) {
        throw tf2::TransformException("setpoint frame mismatch after TF");
    }

    if (pose.pose.position.z < m_height_low) {
        RCLCPP_WARN(get_logger(), "Setpoint to low %.03fm. Clamp to %.03fm",
                    pose.pose.position.z, m_height_low);
        pose.pose.position.z = m_height_low;
    }
}

void offboard::set_yaw(geometry_msgs::msg::Quaternion& q, double yaw) const {
    tf2::Quaternion quat;
    quat.setRPY(0.0, 0.0, yaw);
    q = tf2::toMsg(quat);
}

void offboard::on_tf_failed(const std::string& detail) {
    try {
        m_fsm.handle_event(offboard_fsm::event::tf_failed, m_fcu, m_nav,
                           m_clock->now(), detail);
    } catch (const std::exception& ex) {
        RCLCPP_WARN(get_logger(), "FSM tf_failed handling: %s", ex.what());
    }
}

void offboard::publish_offboard() {
    if (!m_fcu.ready() && m_fsm.current_state() == offboard_fsm::state::idle) {
        return;
    }

    geometry_msgs::msg::PoseStamped current_pose = m_pose_setpoint;
    try {
        current_pose = m_fcu.get_pose();
    } catch (const std::exception& ex) {
        RCLCPP_WARN_THROTTLE(get_logger(), *m_clock, 2000,
                             "get_pose failed: %s", ex.what());
        if (m_fsm.current_state() == offboard_fsm::state::idle) {
            return;
        }
    }

    geometry_msgs::msg::PoseStamped out{};
    m_fsm.tick(m_fcu, m_nav, current_pose, m_clock->now(), out);

    m_pose_setpoint = out;

    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double yaw = 0.0;
    extract_pose(out, x, y, z, yaw);
    m_fcu.set_position_setpoint(x, y, z, yaw);

    RCLCPP_DEBUG_THROTTLE(get_logger(), *m_clock, 1000,
                          "publish pose: x: %.02f y: %.02f z: %.02f yaw: %.02f",
                          x, y, z, yaw);

    if (m_process_callback) {
        m_process_callback();
    }
}

void offboard::nav_current_diff(tf2::Vector3& diff_pos,
                                double& diff_yaw) const {
    const auto current = m_fcu.get_pose();
    navigation_controller::pose_diff(current, m_fsm.command_pose(), diff_pos,
                                     diff_yaw);
}

}  // namespace clover2_fcu_bridge
