#include <clover2_aruco/tracker.hpp>
#include <lifecycle_msgs/msg/state.hpp>
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_msgs/msg/tf_message.hpp>

#include <fstream>

namespace clover2_aruco {

tracker::tracker(const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
    : clover2_common::lifecycle_node("tracker", options) {
    enable_watch_parameters();

    declare_and_watch_parameter<std::string>(
        "odom", "odom",
        [this](const rclcpp::Parameter& p) { m_odom_id = p.as_string(); },
        "Odometry frame_id");

    declare_and_watch_parameter<std::string>(
        "tracking", "track",
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

    try {
        m_map_client =
            std::make_shared<clover2_aruco::map_client>(shared_from_this());
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Map client creation fail: %s", e.what());
        return tracker::CallbackReturn::ERROR;
    }

    m_markers_sub = create_subscription<clover2_aruco_msgs::msg::MarkerArray>(
        "~/markers", rclcpp::SensorDataQoS(),
        std::bind(&tracker::markers_callback, this, std::placeholders::_1));

    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_markers_sub.reset();

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
    geometry_msgs::msg::TransformStamped t;

    try {
        t = m_tf_buffer->lookupTransform(m_tracking_id, msg->header.frame_id,
                                         tf2::TimePointZero);
    } catch (const tf2::TransformException& ex) {
        RCLCPP_ERROR(get_logger(), "Unable got transform %s to %s: %s",
                     toFrameRel.c_str(), fromFrameRel.c_str(), ex.what());
        return;
    }

    tf2::doTransform(msg->markers[0].pose, msg->markers[0].pose, t);
}

}  // namespace clover2_aruco
