#include <clover2_aruco/tracker.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_msgs/msg/tf_message.hpp>

#include <fstream>

namespace clover2_aruco {

tracker::tracker(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("tracker", options) {
    enable_watch_parameters();

    declare_and_watch_parameter<std::string>(
        "odom", "map",
        [this](const rclcpp::Parameter& p) { m_odom_id = p.as_string(); },
        "Odometry frame_id");

    declare_and_watch_parameter<std::string>(
        "tracking", "base_link",
        [this](const rclcpp::Parameter& p) { m_tracking_id = p.as_string(); },
        "Tracking result");

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
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(*this);
    m_tf_buffer = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    m_tf_listener = std::make_shared<tf2_ros::TransformListener>(*m_tf_buffer);

    m_map_client = std::make_shared<map_client>(shared_from_this());

    m_pose_pub = create_publisher<geometry_msgs::msg::PoseStamped>(
        "~/pose", rclcpp::SensorDataQoS());

    m_markers_sub = create_subscription<clover2_aruco_msgs::msg::MarkerArray>(
        "~/markers", rclcpp::SensorDataQoS(),
        std::bind(&tracker::markers_callback, this, std::placeholders::_1));

    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_markers_sub.reset();
    m_pose_pub.reset();

    m_map_client.reset();

    m_tf_listener.reset();
    m_tf_buffer.reset();
    m_tf_broadcaster.reset();

    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    return tracker::CallbackReturn::SUCCESS;
}

void tracker::markers_callback(
    const clover2_aruco_msgs::msg::MarkerArray::SharedPtr msg) {
    if (msg->markers.size() == 0) {
        return;
    }

    std::lock_guard<map_client> map_guard(*m_map_client);

    geometry_msgs::msg::TransformStamped camera_transform;

    try {
        camera_transform = m_tf_buffer->lookupTransform(
            m_tracking_id, msg->header.frame_id, tf2::TimePointZero);
    } catch (const tf2::TransformException& ex) {
        RCLCPP_ERROR(get_logger(), "Unable got transform %s to %s: %s",
                     m_tracking_id.c_str(), msg->header.frame_id.c_str(),
                     ex.what());
        return;
    }

    // // get markers in tracking frame
    transform_marker(msg->markers, camera_transform);

    // get markers
    geometry_msgs::msg::TransformStamped marker_map_transform;
    for (auto& marker : msg->markers) {
        if (!m_map_client->has_marker(marker.id)) {
            continue;
        }

        // if (!m_tf_buffer->canTransform(
        //         m_map_client->get_marker_frame_id(marker.id),
        //         m_map_client->get_map_id(), rclcpp::Time(0),
        //         rclcpp::Duration::from_nanoseconds(1000))) {
        //     throw std::runtime_error();
        // }

        marker_map_transform = m_tf_buffer->lookupTransform(
            "map_"+m_map_client->get_marker_frame_id(marker.id),
            m_map_client->get_map_id(), tf2::TimePointZero);

        transform_marker(marker, marker_map_transform);
    }

    geometry_msgs::msg::PoseStamped estimated_pose;
    estimated_pose.header.stamp = msg->header.stamp;
    estimated_pose.header.frame_id = m_map_client->get_map_id();

    estimated_pose.pose = msg->markers[0].pose;
    tf2::Transform t;
    tf2::fromMsg(msg->markers[0].pose, t);
    tf2::toMsg(t.inverse(), estimated_pose.pose);

    m_pose_pub->publish(estimated_pose);
}

void tracker::transform_marker(
    std::vector<clover2_aruco_msgs::msg::Marker>& markers,
    const geometry_msgs::msg::TransformStamped& t) {
    for (auto& marker : markers) {
        transform_marker(marker, t);
    }
}

void tracker::transform_marker(clover2_aruco_msgs::msg::Marker& marker,
                               const geometry_msgs::msg::TransformStamped& t) {
    tf2::doTransform(marker.pose, marker.pose, t);
    tf2::doTransform(marker.transform, marker.transform, t);
}

}  // namespace clover2_aruco

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_aruco::tracker)
