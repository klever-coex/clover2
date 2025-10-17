#pragma once

// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// Clover2 includes
#include <clover2_aruco/map_client.hpp>
#include <clover2_aruco/utils/ekf.hpp>
#include <clover2_common/lifecycle_node.hpp>

// TF2 includes
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>

// OpenCV includes
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/dictionary.hpp>
#include <opencv2/core.hpp>

// Msgs includes
#include <clover2_aruco_msgs/msg/marker.hpp>
#include <clover2_aruco_msgs/msg/marker_array.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>


namespace clover2_aruco {

class tracker : public clover2_common::lifecycle_node {
public:
    using SharedPtr =
        std::shared_ptr<tracker>;
    using CallbackReturn =
        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::
            CallbackReturn;
    using SetParametersResult =
        rcl_interfaces::msg::SetParametersResult;

    explicit tracker(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    CallbackReturn on_configure(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_activate(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& /* state */);

    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& /* state */);

private:
    // Camera parameters

    std::string m_aruco_frame_id;  ///< Base frame for ArUco markers
    std::mutex m_camera_info_mtx;  ///< Mutex for thread-safe camera info access

    // Detection parameters
    
    std::shared_ptr<map_client>
        m_map_client;  ///< Map client for marker metadata

    // TF
    std::shared_ptr<tf2_ros::TransformBroadcaster>
        m_tf_broadcaster;  ///< TF broadcaster

    // Publishers and subscribers
    rclcpp::Publisher<clover2_aruco_msgs::msg::MarkerArray>::SharedPtr
        m_markers_pub;  ///< Marker array publisher
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr
        m_debug_pub;  ///< Debug image publisher
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;  ///< Camera info subscriber
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr
        m_image_sub;  ///< Camera image subscriber
};

}  // namespace clover2_aruco
