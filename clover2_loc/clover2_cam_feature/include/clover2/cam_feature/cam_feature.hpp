#pragma once

// clover2
#include <clover2/cam_feature/base_plugin.hpp>
#include <clover2/cam_feature/cam_feature_diagnostics.hpp>
#include <clover2/map/client.hpp>
#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node.hpp>

// ROS 2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <image_geometry/pinhole_camera_model.hpp>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/executor.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>

// msgs
#include <clover2_pose_msgs/msg/marker_array.hpp>

// STL
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2::cam_feature {

class cam_feature : public clover2_common::lifecycle_node {
public:
    using SharedPtr = std::shared_ptr<cam_feature>;
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::
        LifecycleNodeInterface::CallbackReturn;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit cam_feature(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    virtual ~cam_feature();

    CallbackReturn on_configure(const rclcpp_lifecycle::State& /* state */);
    CallbackReturn on_activate(const rclcpp_lifecycle::State& /* state */);
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& /* state */);
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& /* state */);
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& /* state */);

private:
    /**
     * @brief Callback for image topic subscription.
     * @param msg Incoming camera image
     */
    void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr msg);

    /**
     * @brief Callback for camera info topic subscription.
     * @param msg Incoming camera info message
     */
    void camera_info_callback(
        const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg);

    void produce_camera_info_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& stat);
    void produce_map_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& stat);
    void produce_marker_hz_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& stat);

    std::mutex m_camera_info_mtx;
    image_geometry::PinholeCameraModel m_camera_model;

    std::vector<std::string> m_plugin_ids;
    std::vector<std::string> m_default_plugin_ids{"aruco"};
    pluginlib::ClassLoader<base_plugin> m_plugin_loader{
        "clover2_cam_feature", "clover2::cam_feature::base_plugin"};
    std::unordered_map<std::string, base_plugin::SharedPtr> m_plugins;

    std::shared_ptr<clover2::map::client> m_map_client;

    rclcpp::Publisher<clover2_pose_msgs::msg::MarkerArray>::SharedPtr
        m_markers_pub;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_image_debug_pub;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;

    rclcpp::Time m_last_camera_info_stamp;
    uint32_t m_last_camera_info_width{0};
    uint32_t m_last_camera_info_height{0};
    size_t m_processed_frames{0};

    double m_min_marker_hz{25.0};
    rclcpp::Time m_last_marker_hz_stamp;
    size_t m_last_marker_processed_frames{0};
    double m_marker_hz{0.0};
};

}  // namespace clover2::cam_feature
