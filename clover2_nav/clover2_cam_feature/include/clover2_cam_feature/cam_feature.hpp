#pragma once

// clover2
#include <clover2/common/lifecycle_node.hpp>
#include <clover2/common/parameter_watcher.hpp>
#include <clover2/map_server/map_client.hpp>
#include <clover2_cam_feature/base_plugin.hpp>

// ROS 2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <image_geometry/pinhole_camera_model.hpp>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>

// msgs
#include <clover2_localization_msgs/msg/feature_array.hpp>

// STL
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace clover2_cam_feature {

class cam_feature : public clover2::common::lifecycle_node {
public:
    using SharedPtr = std::shared_ptr<cam_feature>;
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::
        LifecycleNodeInterface::CallbackReturn;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit cam_feature(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~cam_feature() override;

    /**
     * @brief Lifecycle callback: configure the node.
     */
    CallbackReturn on_configure(
        const rclcpp_lifecycle::State& /* state */) override;

    /**
     * @brief Lifecycle callback: activate the node.
     */
    CallbackReturn on_activate(
        const rclcpp_lifecycle::State& /* state */) override;

    /**
     * @brief Lifecycle callback: deactivate the node.
     */
    CallbackReturn on_deactivate(
        const rclcpp_lifecycle::State& /* state */) override;

    /**
     * @brief Lifecycle callback: cleanup resources.
     */
    CallbackReturn on_cleanup(
        const rclcpp_lifecycle::State& /* state */) override;

    /**
     * @brief Lifecycle callback: shutdown resources.
     */
    CallbackReturn on_shutdown(
        const rclcpp_lifecycle::State& /* state */) override;

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

    /**
     * @brief Produce diagnostics information for the node.
     * @param stat Diagnostic status wrapper
     */
    void produce_diagnostics(diagnostic_updater::DiagnosticStatusWrapper& stat);

    CallbackReturn load_plugins();

    std::mutex m_camera_info_mtx;
    image_geometry::PinholeCameraModel m_camera_model;

    std::vector<std::string> m_plugin_ids;
    std::vector<std::string> m_plugin_types;
    std::unique_ptr<pluginlib::ClassLoader<base_plugin>> m_plugin_loader;
    std::vector<base_plugin::SharedPtr> m_plugins;

    std::shared_ptr<clover2::map_server::map_client> m_map_client;

    size_t m_last_pose_count;
    clover2::common::parameter_watcher::SharedPtr m_parameter_watcher;
    std::shared_ptr<diagnostic_updater::Updater> m_diagnostic_updater;

    rclcpp::Publisher<clover2_localization_msgs::msg::FeatureArray>::SharedPtr
        m_features_pub;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_image_debug_pub;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;
};

}  // namespace clover2_cam_feature
