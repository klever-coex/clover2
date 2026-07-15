#pragma once

// clover2
#include <clover2/map/io/fs_provider.hpp>
#include <clover2/map/server_diagnostics.hpp>
#include <clover2_common/node.hpp>

// ROS2
#include <clover2_pose_msgs/msg/marker_map.hpp>
#include <clover2_pose_msgs/srv/get_map.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/empty.hpp>
#include <tf2_ros/static_transform_broadcaster.hpp>

// STL
#include <filesystem>
#include <memory>
#include <mutex>

namespace clover2::map {

class server : public clover2_common::node {
public:
    using SharedPtr = std::shared_ptr<server>;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit server(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
    void map_callback(
        const clover2_pose_msgs::srv::GetMap::Request::SharedPtr request,
        clover2_pose_msgs::srv::GetMap::Response::SharedPtr response);

    void update_map();

    void update_diagnostic_map_state();

    void produce_map_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& stat);
    void produce_interface_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& stat);

    std::recursive_mutex m_map_mtx;
    // std::filesystem::path m_map_path;
    std::shared_ptr<io::fs_provider> m_provider;

    std::shared_ptr<tf2_ros::StaticTransformBroadcaster>
        m_tf_static_broadcaster;

    rclcpp::TimerBase::SharedPtr m_start_timer;

    rclcpp::Service<clover2_pose_msgs::srv::GetMap>::SharedPtr m_map_service;
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr m_map_update_pub;

    bool m_map_loaded{false};
    std::string m_map_frame_id;
    std::string m_map_path;
    size_t m_marker_count{0};
    size_t m_get_map_requests{0};
    size_t m_map_updates{0};
    size_t m_static_tf_count{0};
};

}  // namespace clover2::map
