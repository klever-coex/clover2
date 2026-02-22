// clover2
#include <clover2/offboard/bridge/mavros_bridge.hpp>

// ROS2
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <mavros_msgs/msg/position_target.hpp>
#include <mavros_msgs/msg/state.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/set_mode.hpp>

// Eigen
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>

namespace clover2::offboard::bridge {

namespace {
constexpr int FRAME_LOCAL_NED = 1;
constexpr uint16_t IGNORE_PX = 1 << 0;
constexpr uint16_t IGNORE_PY = 1 << 1;
constexpr uint16_t IGNORE_PZ = 1 << 2;
constexpr uint16_t IGNORE_VX = 1 << 3;
constexpr uint16_t IGNORE_VY = 1 << 4;
constexpr uint16_t IGNORE_VZ = 1 << 5;
constexpr uint16_t IGNORE_AFX = 1 << 6;
constexpr uint16_t IGNORE_AFY = 1 << 7;
constexpr uint16_t IGNORE_AFZ = 1 << 8;
constexpr uint16_t IGNORE_YAW = 1 << 10;
constexpr uint16_t IGNORE_YAW_RATE = 1 << 11;
}  // namespace

mavros_bridge::mavros_bridge(const creation_context& ctx) : base_bridge(ctx) {
    auto qos = rclcpp::QoS(rclcpp::KeepLast(10))
                   .reliability(rclcpp::ReliabilityPolicy::BestEffort)
                   .durability(rclcpp::DurabilityPolicy::Volatile);

    m_setpoint_pub = m_node->create_publisher<mavros_msgs::msg::PositionTarget>(
        "mavros/setpoint_raw/local", 10);

    m_set_mode_client =
        m_node->create_client<mavros_msgs::srv::SetMode>("mavros/set_mode");
    m_arming_client =
        m_node->create_client<mavros_msgs::srv::CommandBool>("mavros/cmd/arming");

    m_state_sub = m_node->create_subscription<mavros_msgs::msg::State>(
        "mavros/state", qos,
        std::bind(&mavros_bridge::state_callback, this, std::placeholders::_1));
    m_pose_sub =
        m_node->create_subscription<geometry_msgs::msg::PoseStamped>(
            "mavros/local_position/pose", qos,
            std::bind(&mavros_bridge::pose_callback, this,
                      std::placeholders::_1));
    m_velocity_sub =
        m_node->create_subscription<geometry_msgs::msg::TwistStamped>(
            "mavros/local_position/velocity_local", qos,
            std::bind(&mavros_bridge::velocity_callback, this,
                      std::placeholders::_1));
}

void mavros_bridge::state_callback(const mavros_msgs::msg::State::SharedPtr msg) {
    std::lock_guard lock(m_state_mutex);
    m_state = *msg;
}

void mavros_bridge::pose_callback(
    const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
    std::lock_guard lock(m_state_mutex);
    m_pose = *msg;

    m_current_position.x() = msg->pose.position.x;
    m_current_position.y() = msg->pose.position.y;
    m_current_position.z() = msg->pose.position.z;

    tf2::Quaternion q(msg->pose.orientation.x, msg->pose.orientation.y,
                      msg->pose.orientation.z, msg->pose.orientation.w);
    tf2::Matrix3x3 m(q);
    double roll, pitch, yaw;
    m.getRPY(roll, pitch, yaw);
    m_current_orientation_rpy.x() = roll;
    m_current_orientation_rpy.y() = pitch;
    m_current_orientation_rpy.z() = yaw;

    m_current_orientation = Eigen::Quaterniond(
        msg->pose.orientation.w, msg->pose.orientation.x,
        msg->pose.orientation.y, msg->pose.orientation.z);
}

void mavros_bridge::velocity_callback(
    const geometry_msgs::msg::TwistStamped::SharedPtr msg) {
    std::lock_guard lock(m_state_mutex);
    m_velocity = *msg;
}

std::string mavros_bridge::get_mode() const {
    std::lock_guard lock(m_state_mutex);
    if (m_state) {
        return m_state->mode;
    }
    return "";
}

void mavros_bridge::set_local_position_setpoint(const Eigen::Vector3d& position,
                                                double yaw) {
    mavros_msgs::msg::PositionTarget msg;
    msg.header.stamp = m_node->get_clock()->now();
    msg.header.frame_id = "map";
    msg.coordinate_frame = FRAME_LOCAL_NED;
    msg.type_mask = IGNORE_AFX | IGNORE_AFY | IGNORE_AFZ | IGNORE_YAW_RATE;
    msg.position.x = position.x();
    msg.position.y = position.y();
    msg.position.z = position.z();
    msg.yaw = yaw;

    Eigen::Vector3d current_pos;
    {
        std::lock_guard lock(m_state_mutex);
        current_pos = m_current_position;
    }
    Eigen::Vector3d delta = position - current_pos;
    const double dist = delta.norm();
    if (dist > 1e-6 && m_speed_limit > 0) {
        const double vel_mag = std::min(dist, m_speed_limit);
        Eigen::Vector3d velocity = delta * (vel_mag / dist);
        msg.velocity.x = velocity.x();
        msg.velocity.y = velocity.y();
        msg.velocity.z = velocity.z();
    } else {
        msg.type_mask |= IGNORE_VX | IGNORE_VY | IGNORE_VZ;
    }

    m_setpoint_pub->publish(msg);
}

void mavros_bridge::set_local_velocity_setpoint(const Eigen::Vector3d& velocity,
                                               double yaw_rate) {
    mavros_msgs::msg::PositionTarget msg;
    msg.header.stamp = m_node->get_clock()->now();
    msg.header.frame_id = "map";
    msg.coordinate_frame = FRAME_LOCAL_NED;
    msg.type_mask = IGNORE_PX | IGNORE_PY | IGNORE_PZ | IGNORE_AFX |
                   IGNORE_AFY | IGNORE_AFZ | IGNORE_YAW;
    msg.velocity.x = velocity.x();
    msg.velocity.y = velocity.y();
    msg.velocity.z = velocity.z();
    msg.yaw_rate = yaw_rate;
    m_setpoint_pub->publish(msg);
}

void mavros_bridge::set_mode(const std::string& mode) {
    auto request = std::make_shared<mavros_msgs::srv::SetMode::Request>();
    request->custom_mode = mode;
    m_set_mode_client->async_send_request(request);
}

void mavros_bridge::set_offboard_controller(data::controller_type type) {
    (void)type;
    set_mode("OFFBOARD");
}

void mavros_bridge::set_speed(double speed) {
    std::lock_guard lock(m_state_mutex);
    m_speed_limit = std::max(0.0, speed);
}

void mavros_bridge::arm() {
    auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
    request->value = true;
    m_arming_client->async_send_request(request);
}

void mavros_bridge::disarm() {
    auto request = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
    request->value = false;
    m_arming_client->async_send_request(request);
}

}  // namespace clover2::offboard::bridge
