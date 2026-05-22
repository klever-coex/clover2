#include <clover2/aruco/tracker.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_eigen/tf2_eigen.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_msgs/msg/tf_message.hpp>

#include <fstream>

namespace clover2::aruco {

tracker::tracker(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("tracker", options) {
    declare_and_watch_parameter<std::string>(
        "frame_id", "base_link",
        [this](const rclcpp::Parameter& p) { m_frame_id = p.as_string(); },
        "Tracking target");

    declare_and_watch_parameter<bool>(
        "tf.send", true,
        [this](const rclcpp::Parameter& p) { m_tf_send = p.as_bool(); },
        "Send transform");

    declare_and_watch_parameter<std::string>(
        "tf.child_frame_id", "vision_estimate",
        [this](const rclcpp::Parameter& p) {
            m_child_frame_id = p.as_string();
        },
        "Vision position estimate frame");

    declare_and_watch_parameter<double>(
        "xy_variation", 0.4,
        [this](const rclcpp::Parameter& p) {
            m_xy_variation = p.as_string();
        },
        "Published variation for x and y");

    declare_and_watch_parameter<double>(
        "z_variation", 0.4,
        [this](const rclcpp::Parameter& p) {
            m_z_variation = p.as_string();
        },
        "Published variation for x and y");

    register_on_configure(
        std::bind(&tracker::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&tracker::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&tracker::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&tracker::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&tracker::on_shutdown, this, std::placeholders::_1));
}

tracker::~tracker() {}

tracker::CallbackReturn tracker::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_callback_group =
        create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    try {
        m_map_client = std::make_shared<clover2::map::client>(
            shared_from_this(), m_callback_group);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Fail to create map client. Exception: %s",
                     e.what());
        on_cleanup(state);
        return CallbackReturn::FAILURE;
    }

    RCLCPP_INFO(get_logger(), "Configured");
    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(*this);
    m_tf_buffer = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    m_tf_listener = std::make_shared<tf2_ros::TransformListener>(*m_tf_buffer);

    m_pose_pub = create_publisher<geometry_msgs::msg::PoseStamped>(
        "~/pose", rclcpp::SystemDefaultsQoS());

    m_pose_cov_pub =
        create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>(
            "~/pose_cov", rclcpp::SystemDefaultsQoS());

    m_poses_debug_pub = create_publisher<geometry_msgs::msg::PoseArray>(
        "~/poses_debug", rclcpp::SystemDefaultsQoS());

    rclcpp::SubscriptionOptions options;
    options.callback_group = m_callback_group;

    m_markers_sub = create_subscription<clover2_pose_msgs::msg::MarkerArray>(
        "~/markers", rclcpp::SensorDataQoS(),
        std::bind(&tracker::markers_callback, this, std::placeholders::_1),
        options);

    RCLCPP_INFO(get_logger(), "Activated");
    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_markers_sub.reset();
    m_pose_pub.reset();
    m_pose_cov_pub.reset();
    m_poses_debug_pub.reset();

    m_tf_listener.reset();
    m_tf_buffer.reset();
    m_tf_broadcaster.reset();

    RCLCPP_INFO(get_logger(), "Deactivated");
    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_map_client.reset();

    RCLCPP_INFO(get_logger(), "Cleaned up");
    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    return tracker::CallbackReturn::SUCCESS;
}

void tracker::markers_callback(
    const clover2_pose_msgs::msg::MarkerArray::SharedPtr msg) {
    if (msg->markers.size() == 0) {
        return;
    }

    // transform between tracking frame and camera frame
    Eigen::Isometry3d camera_transform;
    try {
        auto camera_transform_msg = m_tf_buffer->lookupTransform(
            m_frame_id, msg->header.frame_id, tf2::TimePointZero);

        camera_transform = tf2::transformToEigen(camera_transform_msg);
    } catch (const tf2::TransformException& ex) {
        RCLCPP_ERROR(get_logger(), "Unable got transform %s to %s: %s",
                     m_frame_id.c_str(), msg->header.frame_id.c_str(),
                     ex.what());
        return;
    }

    // tracking position
    geometry_msgs::msg::PoseStamped estimated_pose;
    estimated_pose.header.stamp = msg->header.stamp;
    estimated_pose.header.frame_id = m_map_client->get_map_id();

    geometry_msgs::msg::PoseWithCovarianceStamped estimated_pose_cov;
    estimated_pose_cov.header = estimated_pose.header;

    // debug poses of camera from each marker
    geometry_msgs::msg::PoseArray poses_debug;
    poses_debug.header.stamp = msg->header.stamp;
    poses_debug.header.frame_id = m_map_client->get_map_id();
    poses_debug.poses.reserve(msg->markers.size());

    // temp variables for estimating
    Eigen::Vector3d avg_translation = Eigen::Vector3d::Zero();
    Eigen::Quaterniond avg_quat = Eigen::Quaterniond::Identity();
    Eigen::Vector4d cumulative_q = Eigen::Vector4d::Zero();

    for (const auto& marker : msg->markers) {
        Eigen::Isometry3d marker_pose = Eigen::Isometry3d::Identity();
        tf2::fromMsg(marker.pose.pose, marker_pose);

        Eigen::Isometry3d camera_in_map =
            m_map_client->get_transform(marker.id) * marker_pose.inverse();

        Eigen::Isometry3d drone_in_map = camera_in_map * camera_transform;

        // add debug transform
        poses_debug.poses.push_back(tf2::toMsg(drone_in_map));

        // estimate position
        avg_translation += drone_in_map.translation();
        Eigen::Quaterniond q(drone_in_map.rotation());
        cumulative_q += q.coeffs();
    }

    // finalize pose estimation
    avg_translation /= static_cast<double>(msg->markers.size());
    cumulative_q /= static_cast<double>(msg->markers.size());
    avg_quat.coeffs() = cumulative_q.normalized();

    // fill pose msg
    Eigen::Isometry3d result_pose = Eigen::Isometry3d::Identity();
    result_pose.translate(avg_translation);
    result_pose.rotate(avg_quat);

    estimated_pose.pose = tf2::toMsg(result_pose);
    estimated_pose_cov.pose.pose = estimated_pose.pose;
    estimated_pose_cov.pose.covariance[0] = m_xy_variation;
    estimated_pose_cov.pose.covariance[7] = m_xy_variation;
    estimated_pose_cov.pose.covariance[14] = m_z_variation;

    estimated_pose_cov.pose.covariance[21] = 0.3;
    estimated_pose_cov.pose.covariance[28] = 0.3;
    estimated_pose_cov.pose.covariance[35] = 0.1;

    // publish estimated pose
    m_pose_pub->publish(estimated_pose);
    m_pose_cov_pub->publish(estimated_pose_cov);

    publish_tf(estimated_pose.header, result_pose.inverse());

    // publish tracker id poses form each marker
    if (m_poses_debug_pub->get_subscription_count() != 0) {
        m_poses_debug_pub->publish(poses_debug);
    }
}

void tracker::publish_tf(const std_msgs::msg::Header& header,
                         Eigen::Isometry3d pose) {
    if (m_tf_send) {
        geometry_msgs::msg::TransformStamped transform =
            tf2::eigenToTransform(pose);

        transform.header.stamp = header.stamp;
        transform.header.frame_id = m_child_frame_id;
        transform.child_frame_id = header.frame_id;

        m_tf_broadcaster->sendTransform(transform);
    }
}

}  // namespace clover2::aruco

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::aruco::tracker)
