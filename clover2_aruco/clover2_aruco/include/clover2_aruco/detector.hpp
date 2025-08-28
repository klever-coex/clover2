#pragma once

// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

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

class detector : public rclcpp_lifecycle::LifecycleNode {
public:
    using SharedPtr = std::shared_ptr<detector>;
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::
        LifecycleNodeInterface::CallbackReturn;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit detector(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    CallbackReturn on_configure(const rclcpp_lifecycle::State& state);
    CallbackReturn on_activate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& state);
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& state);

private:
    void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr msg);
    void camera_info_callback(
        const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg);

    SetParametersResult on_set_parameters_cb(
        const std::vector<rclcpp::Parameter>& parameters);

    void fill_corners(clover2_aruco_msgs::msg::Marker& marker,
                      const std::vector<cv::Point2f>& corners) const;
    void fill_pose(clover2_aruco_msgs::msg::Marker& marker,
                   const cv::Vec3d& rvec, const cv::Vec3d& tvec) const;

    std::mutex m_camera_info_mtx;
    cv::Mat m_camera_matrix;
    cv::Mat m_marker_obj_points;
    cv::Mat m_distortion_coeffs;

    int m_dictionary_id;
    double m_marker_size;
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
    cv::Ptr<cv::aruco::DetectorParameters> m_detector_parameters;

    std::shared_ptr<tf2_ros::Buffer> m_tf_buffer;
    std::shared_ptr<tf2_ros::TransformListener> m_tf_listener;
    std::shared_ptr<tf2_ros::TransformBroadcaster> m_tf_broadcaster;

    rclcpp::Node::OnSetParametersCallbackHandle::SharedPtr
        m_set_parameters_handle_ptr;

    rclcpp::Publisher<clover2_aruco_msgs::msg::MarkerArray>::SharedPtr
        m_markers_pub;
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;
};

}  // namespace clover2_aruco
