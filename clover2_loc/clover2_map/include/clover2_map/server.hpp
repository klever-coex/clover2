#pragma once

// clover2
#include <clover2_map/io/fs_provider.hpp>
#include <clover2_common/node.hpp>

// ROS2
#include <clover2_pose_msgs/msg/marker_map.hpp>
#include <clover2_pose_msgs/srv/get_map.hpp>
#include <clover2_pose_msgs/srv/modify_map.hpp>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/empty.hpp>
#include <tf2_ros/transform_broadcaster.hpp>

// STL
#include <memory>

namespace clover2_map {

class server : public clover2_common::node {
public:
    using SharedPtr = std::shared_ptr<server>;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit server(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

private:
    void map_callback(
        const clover2_pose_msgs::srv::GetMap::Request::SharedPtr request,
        clover2_pose_msgs::srv::GetMap::Response::SharedPtr response);

    void modify_map_callback(
        const clover2_pose_msgs::srv::ModifyMap::Request::SharedPtr request,
        clover2_pose_msgs::srv::ModifyMap::Response::SharedPtr response);

    void publish_tf_snapshot();

    void update_map();

    // Concurrency model: every callback that touches the map (the map
    // parameter watcher, both services and the TF timer) runs in the
    // node's default MutuallyExclusive callback group, which serializes
    // them regardless of the executor. Do not create additional callback
    // groups and do not add a mutex without breaking this invariant.
    std::shared_ptr<io::fs_provider> m_provider;

    bool m_send_tf{true};
    std::shared_ptr<tf2_ros::TransformBroadcaster> m_tf_broadcaster;
    rclcpp::TimerBase::SharedPtr m_tf_timer;

    rclcpp::Service<clover2_pose_msgs::srv::GetMap>::SharedPtr m_map_service;
    rclcpp::Service<clover2_pose_msgs::srv::ModifyMap>::SharedPtr
        m_modify_map_service;
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr m_map_update_pub;
};

}  // namespace clover2_map
