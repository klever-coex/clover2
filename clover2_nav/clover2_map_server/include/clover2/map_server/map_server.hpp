#pragma once

#include <clover2/common/lifecycle_node.hpp>
#include <clover2/common/parameter_watcher.hpp>

#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>
#include <tf2_ros/static_transform_broadcaster.hpp>

#include <clover2_aruco_msgs/msg/marker_map.hpp>
#include <std_msgs/msg/empty.hpp>

#include <clover2_aruco_msgs/srv/get_map.hpp>

#include <filesystem>
#include <memory>
#include <mutex>

namespace clover2::map_server {

class map_server : public clover2::common::lifecycle_node {
public:
    using SharedPtr = std::shared_ptr<map_server>;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit map_server(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    CallbackReturn on_configure(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_activate(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& /* state */);

private:
    void map_callback(
        const clover2_aruco_msgs::srv::GetMap::Request::SharedPtr request,
        clover2_aruco_msgs::srv::GetMap::Response::SharedPtr response);

    clover2_aruco_msgs::msg::MarkerMap::SharedPtr parse_map(
        const std::filesystem::path& filename) const;

    void update_map(const std::filesystem::path& filename);

    void update_map(clover2_aruco_msgs::msg::MarkerMap::SharedPtr new_map);

    std::recursive_mutex m_map_mtx;
    std::filesystem::path m_map_path;
    clover2_aruco_msgs::msg::MarkerMap::SharedPtr m_map_msg;

    std::shared_ptr<tf2_ros::StaticTransformBroadcaster> m_tf_static_broadcaster;

    rclcpp::TimerBase::SharedPtr m_map_notify_timer;

    clover2::common::parameter_watcher::SharedPtr m_parameter_watcher;

    rclcpp::Service<clover2_aruco_msgs::srv::GetMap>::SharedPtr m_map_service;
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr m_map_update_pub;
};

}  // namespace clover2::map_server
