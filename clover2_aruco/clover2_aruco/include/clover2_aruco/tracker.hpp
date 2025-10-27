#pragma once

// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// Clover2 includes
#include <clover2_aruco/map_client.hpp>
#include <clover2_common/lifecycle_node.hpp>

// TF2 includes
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>

// Msgs includes
#include <clover2_aruco_msgs/msg/marker.hpp>
#include <clover2_aruco_msgs/msg/marker_array.hpp>

namespace clover2_aruco {

class tracker : public clover2_common::lifecycle_node {
public:
    using SharedPtr = std::shared_ptr<tracker>;
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::
        LifecycleNodeInterface::CallbackReturn;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit tracker(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    virtual ~tracker();

    CallbackReturn on_configure(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_activate(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& /* state */);

private:
    void markers_callback(
        const clover2_aruco_msgs::msg::MarkerArray::SharedPtr msg);

    void mean_fusion_policy(const std::vector<clover2_aruco_msgs::msg::Marker>& makers, const geometry_msgs::msg::TransformStamped& t, geometry_msgs::msg::PoseStamped& pose);

    // Camera parameters
    std::string m_odom_id;      ///< Odometry frame_id
    std::string m_tracking_id;  ///< Tracking result frame_id

    // Detection parameters
    std::shared_ptr<map_client>
        m_map_client;  ///< Map client for marker metadata

    // TF
    std::shared_ptr<tf2_ros::TransformBroadcaster>
        m_tf_broadcaster;                          ///< TF broadcaster
    std::shared_ptr<tf2_ros::Buffer> m_tf_buffer;  ///< TF buffer
    std::shared_ptr<tf2_ros::TransformListener>
        m_tf_listener;  ///< TF transform listener

    // Publishers and subscribers
    rclcpp::Subscription<clover2_aruco_msgs::msg::MarkerArray>::SharedPtr
        m_markers_sub;  ///< Detected markers sub
};

}  // namespace clover2_aruco
