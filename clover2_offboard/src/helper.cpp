// clover2
#include <clover2_offboard/backend/fabric.hpp>
#include <clover2_offboard/helper.hpp>

// ROS2
#include <tf2/utils.h>

// STL
#include <chrono>

namespace clover2_offboard {

std::vector<std::string> helper::list_backends() {
    return backend::fabric::instance().list_backends();
}

void helper::set_position(const std::string& frame_id, std::optional<double> x,
                          std::optional<double> y, std::optional<double> z,
                          std::optional<double> yaw) {
    geometry_msgs::msg::PoseStamped pose;

    pose.header.stamp = get_clock()->now();
    pose.header.frame_id = frame_id;

    // pose.pose.position.x
}

void helper::init(const std::string& backend) {
    backend::context ctx(*m_node);
    m_backend = backend::fabric::instance().create(backend, ctx);

    m_tf_buffer = std::make_shared<tf2_ros::Buffer>(m_node->get_clock());
    m_tf_listener =
        std::make_shared<tf2_ros::TransformListener>(*m_tf_buffer, m_node);

    if (m_group.get() == nullptr) {
        m_group = m_node->create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive);
    }

    m_update_timer = m_node->create_wall_timer(
        std::chrono::milliseconds(40),
        [this]() -> void {  //
            publish_offboard();
        },
        m_group, true);

    RCLCPP_INFO(get_logger(), "Backend initialized: %s", backend.c_str());
}

void helper::publish_offboard() {
    switch (m_state) {
        case state::none:
            break;
        case state::position:
            publish_position();
            break;
        default:
            RCLCPP_ERROR(get_logger(), "Unknown state %d", m_state);
            break;
    }
}

void helper::publish_position() {
    double yaw = tf2::getYaw(m_pose_setpoint.pose.orientation);
    m_backend->set_position_setpoint(
        m_pose_setpoint.pose.position.x, m_pose_setpoint.pose.position.y,
        m_pose_setpoint.pose.position.z, yaw);
}

}  // namespace clover2_offboard
