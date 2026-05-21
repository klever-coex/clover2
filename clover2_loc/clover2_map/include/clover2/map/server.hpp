#pragma once

// clover2
#include <clover2_common/parameter_watcher.hpp>
#include <clover2/map/io/fs_provider.hpp>

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

class server : public rclcpp::Node {
public:
    using SharedPtr = std::shared_ptr<server>;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit server(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
    void map_callback(
        const clover2_pose_msgs::srv::GetMap::Request::SharedPtr request,
        clover2_pose_msgs::srv::GetMap::Response::SharedPtr response);

    void update_map();

    clover2_common::parameter_watcher m_parameter_watcher;

    std::recursive_mutex m_map_mtx;
    // std::filesystem::path m_map_path;
    std::shared_ptr<io::fs_provider> m_provider;

    std::shared_ptr<tf2_ros::StaticTransformBroadcaster>
        m_tf_static_broadcaster;

    rclcpp::TimerBase::SharedPtr m_start_timer;

    rclcpp::Service<clover2_pose_msgs::srv::GetMap>::SharedPtr m_map_service;
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr m_map_update_pub;
};

}  // namespace clover2::map
