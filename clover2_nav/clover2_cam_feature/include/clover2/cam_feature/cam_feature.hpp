#pragma once

// clover2
#include <clover2/cam_feature/base_plugin.hpp>
#include <clover2/common/executor.hpp>
#include <clover2/common/node.hpp>
#include <clover2/common/parameter_watcher.hpp>
#include <clover2/map/client.hpp>

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
#include <vector>

namespace clover2::cam_feature {

class cam_feature : public clover2::common::node {
public:
    using SharedPtr = std::shared_ptr<cam_feature>;
    // using CallbackReturn = rclcpp_lifecycle::node_interfaces::
    //     LifecycleNodeInterface::CallbackReturn;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit cam_feature(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    virtual ~cam_feature();

    // CallbackReturn on_configure(const rclcpp_lifecycle::State& /* state */);
    // CallbackReturn on_activate(const rclcpp_lifecycle::State& /* state */);
    // CallbackReturn on_deactivate(const rclcpp_lifecycle::State& /* state */);
    // CallbackReturn on_cleanup(const rclcpp_lifecycle::State& /* state */);
    // CallbackReturn on_shutdown(const rclcpp_lifecycle::State& /* state */);

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

    base_plugin::SharedPtr create_plugin(const std::string& type,
                                         plugin_context& ctx);
    void add_plugin(const std::string& type, plugin_context& ctx);

    void load_plugins();
    void unload_plugins();

    std::mutex m_camera_info_mtx;
    image_geometry::PinholeCameraModel m_camera_model;

    size_t m_last_pose_count{0};
    // std::vector<std::string> m_plugin_ids;
    std::vector<std::string> m_plugin_types;
    std::vector<base_plugin::SharedPtr> m_plugins;
    pluginlib::ClassLoader<plugin_factory> m_plugin_loader;

    std::shared_ptr<clover2::map::client> m_map_client;
    std::shared_ptr<diagnostic_updater::Updater> m_diagnostic_updater;

    std::shared_ptr<clover2::common::executor> m_executor;

    rclcpp::Publisher<clover2_pose_msgs::msg::MarkerArray>::SharedPtr
        m_markers_pub;
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr m_image_debug_pub;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;
};

}  // namespace clover2::cam_feature
