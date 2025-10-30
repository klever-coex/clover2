#pragma once

// ROS2 includes
#include <image_geometry/pinhole_camera_model.hpp>
#include <rclcpp/rclcpp.hpp>

// Clover2 includes
#include <clover2_aruco/map_client.hpp>
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

/**
 * @class detector
 * @brief Lifecycle node for detecting ArUco markers using OpenCV.
 *
 * This node subscribes to camera images and camera info, detects ArUco markers
 * based on a dictionary, publishes marker arrays, and broadcasts transforms.
 */
class detector : public clover2_common::lifecycle_node {
public:
    using SharedPtr =
        std::shared_ptr<detector>;  ///< Shared pointer type for detector
    using CallbackReturn =
        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::
            CallbackReturn;  ///< Lifecycle callback return type
    using SetParametersResult =
        rcl_interfaces::msg::SetParametersResult;  ///< Parameter callback
                                                   ///< result type

    /**
     * @brief Construct a new detector node.
     * @param options Node options for ROS2 configuration
     */
    explicit detector(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    /**
     * @brief Lifecycle callback: configure the node.
     */
    CallbackReturn on_configure(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Lifecycle callback: activate the node.
     */
    CallbackReturn on_activate(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Lifecycle callback: deactivate the node.
     */
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Lifecycle callback: cleanup resources.
     */
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Lifecycle callback: shutdown the node.
     */
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Generate the 3D object points of a marker for pose estimation.
     * @param markerLength Marker side length in meters
     * @param estimate_parameters OpenCV estimation parameters
     * @return cv::Mat 3D points of marker corners
     */
    cv::Mat marker_object_points(
        double markerLength,
        const cv::Ptr<cv::aruco::EstimateParameters>& estimate_parameters);

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
     * @brief Fill marker corners from detected 2D points.
     * @param marker Marker message to fill
     * @param corners 2D corner points
     */
    void fill_corners(clover2_aruco_msgs::msg::Marker& marker,
                      const std::vector<cv::Point2f>& corners) const;

    /**
     * @brief Fill marker pose from rotation and translation vectors.
     * @param marker Marker message to fill
     * @param rvec Rotation vector
     * @param tvec Translation vector
     */
    void fill_pose(clover2_aruco_msgs::msg::Marker& marker,
                   const cv::Vec3d& rvec, const cv::Vec3d& tvec) const;

    /**
     * @brief Fill a geometry_msgs::Vector3 with translation data.
     * @param translation Vector3 message to fill
     * @param tvec Translation vector
     */
    void fill_translation(geometry_msgs::msg::Vector3& translation,
                          const cv::Vec3d& tvec) const;

    /**
     * @brief Get the TF frame ID corresponding to a marker ID.
     * @param id Marker ID
     * @return std::string Marker frame ID
     */
    std::string get_marker_frame_id(const int id) const;

private:
    // Camera parameters
    std::string m_aruco_frame_id;  ///< Base frame for ArUco markers
    std::mutex m_camera_info_mtx;  ///< Mutex for thread-safe camera info access
    image_geometry::PinholeCameraModel m_camera_model;  ///< Camera model

    // Detection parameters
    bool m_tf_publish;
    int m_dictionary_id;   ///< OpenCV ArUco dictionary ID
    double m_marker_size;  ///< Marker size in meters
    std::shared_ptr<map_client>
        m_map_client;  ///< Map client for marker metadata
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;  ///< ArUco dictionary object
    cv::Ptr<cv::aruco::DetectorParameters>
        m_detector_parameters;  ///< OpenCV detector parameters

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
