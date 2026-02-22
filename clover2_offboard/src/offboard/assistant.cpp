// clover2
#include <clover2/offboard/assistant.hpp>

// ROS2
#include <geometry_msgs/msg/point_stamped.hpp>
#include <geometry_msgs/msg/vector3_stamped.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>

// Eigen
#include <Eigen/Dense>

namespace clover2::offboard {

assistant::assistant(rclcpp::Node::SharedPtr node, client& client)
    : m_node(std::move(node)), m_client(&client) {
    m_tf_buffer = std::make_unique<tf2_ros::Buffer>(m_node->get_clock());
    m_tf_listener = std::make_unique<tf2_ros::TransformListener>(
        *m_tf_buffer, *m_node);

    m_timer = m_node->create_wall_timer(
        std::chrono::milliseconds(20),  // 50 Hz
        std::bind(&assistant::timer_callback_50hz, this));
}

void assistant::set_position_setpoint(double x, double y, double z,
                                      double yaw,
                                      const std::string& frame_id) {
    std::lock_guard lock(m_setpoint_mutex);
    m_position_mode = true;
    m_position_x = x;
    m_position_y = y;
    m_position_z = z;
    m_position_yaw = yaw;
    m_frame_id = frame_id;
    m_has_setpoint = true;
}

void assistant::set_speed(double speed) {
    std::lock_guard lock(m_setpoint_mutex);
    m_speed = speed;
}

void assistant::set_velocity_setpoint(double vx, double vy, double vz,
                                     double yaw_rate,
                                     const std::string& frame_id) {
    std::lock_guard lock(m_setpoint_mutex);
    m_position_mode = false;
    m_velocity_x = vx;
    m_velocity_y = vy;
    m_velocity_z = vz;
    m_velocity_yaw_rate = yaw_rate;
    m_frame_id = frame_id;
    m_has_setpoint = true;
}

void assistant::timer_callback_50hz() {
    std::lock_guard lock(m_setpoint_mutex);
    if (!m_has_setpoint) {
        return;
    }

    if (m_position_mode) {
        m_client->set_offboard_controller(
            bridge::data::controller_type::position);
        Eigen::Vector3d position(m_position_x, m_position_y, m_position_z);
        double yaw = m_position_yaw;

        if (m_frame_id != "map") {
            geometry_msgs::msg::PointStamped point_in;
            point_in.header.stamp = m_node->get_clock()->now();
            point_in.header.frame_id = m_frame_id;
            point_in.point.x = m_position_x;
            point_in.point.y = m_position_y;
            point_in.point.z = m_position_z;

            try {
                geometry_msgs::msg::TransformStamped transform =
                    m_tf_buffer->lookupTransform(
                        "map", m_frame_id, tf2::TimePointZero);

                geometry_msgs::msg::PointStamped point_out;
                tf2::doTransform(point_in, point_out, transform);

                position.x() = point_out.point.x;
                position.y() = point_out.point.y;
                position.z() = point_out.point.z;

                if (m_frame_id == "base_link") {
                    double drone_yaw = m_client->get_orientation_rpy().z();
                    yaw = drone_yaw + m_position_yaw;
                }
            } catch (const tf2::TransformException& ex) {
                RCLCPP_WARN_THROTTLE(m_node->get_logger(), *m_node->get_clock(),
                                     1000, "TF transform failed: %s",
                                     ex.what());
                return;
            }
        }

        m_client->set_local_position_setpoint(position, yaw);
    } else {
        m_client->set_offboard_controller(
            bridge::data::controller_type::velocity);
        Eigen::Vector3d velocity(m_velocity_x, m_velocity_y, m_velocity_z);
        if (m_frame_id != "map") {
            geometry_msgs::msg::Vector3Stamped vel_in;
            vel_in.header.stamp = m_node->get_clock()->now();
            vel_in.header.frame_id = m_frame_id;
            vel_in.vector.x = m_velocity_x;
            vel_in.vector.y = m_velocity_y;
            vel_in.vector.z = m_velocity_z;

            try {
                geometry_msgs::msg::TransformStamped transform =
                    m_tf_buffer->lookupTransform(
                        "map", m_frame_id, tf2::TimePointZero);

                geometry_msgs::msg::Vector3Stamped vel_out;
                tf2::doTransform(vel_in, vel_out, transform);

                velocity.x() = vel_out.vector.x;
                velocity.y() = vel_out.vector.y;
                velocity.z() = vel_out.vector.z;
            } catch (const tf2::TransformException& ex) {
                RCLCPP_WARN_THROTTLE(m_node->get_logger(), *m_node->get_clock(),
                                     1000, "TF transform failed: %s",
                                     ex.what());
                return;
            }
        }
        m_client->set_local_velocity_setpoint(velocity, m_velocity_yaw_rate);
    }

    m_client->set_speed(m_speed);
}

}  // namespace clover2::offboard
