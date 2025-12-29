#pragma once

// ROS2 includes
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <image_geometry/pinhole_camera_model.hpp>
#include <rclcpp/rclcpp.hpp>

// Clover2 includes
#include <clover2_common/lifecycle_node.hpp>

// TF2 includes
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/transform_broadcaster.h>

// OpenCV includes
#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

// Msgs includes
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace clover2_odometry {

/**
 * @class mono_visual_odometry
 * @brief Lifecycle node for monocular visual odometry using feature-based tracking.
 *
 * This node subscribes to camera images and camera info, performs visual odometry
 * using FAST feature detector and Lucas-Kanade optical flow, and publishes
 * odometry messages.
 */
class mono_visual_odometry : public clover2_common::lifecycle_node {
public:
    using SharedPtr =
        std::shared_ptr<mono_visual_odometry>;  ///< Shared pointer type
    using CallbackReturn =
        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::
            CallbackReturn;  ///< Lifecycle callback return type
    using SetParametersResult =
        rcl_interfaces::msg::SetParametersResult;  ///< Parameter callback result type

    /**
     * @brief Construct a new mono_visual_odometry node.
     * @param options Node options for ROS2 configuration
     */
    explicit mono_visual_odometry(
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

private:
    /**
     * @brief Detect features in the given image.
     * @param img Input grayscale image
     * @return std::vector<cv::Point2f> Detected feature points
     */
    std::vector<cv::Point2f> detect_features(const cv::Mat& img);

    /**
     * @brief Perform visual odometry between two frames.
     */
    void visual_odometry();

    /**
     * @brief Get adjusted coordinates for publishing.
     * @return cv::Vec3d Adjusted translation vector
     */
    cv::Vec3d get_mono_coordinates() const;

    /**
     * @brief Process a new frame for visual odometry.
     * @param frame New grayscale frame
     */
    void process_frame(const cv::Mat& frame);

    // Camera parameters
    std::mutex m_camera_info_mtx;  ///< Mutex for thread-safe camera info access
    image_geometry::PinholeCameraModel m_camera_model;  ///< Camera model

    // Visual odometry state
    cv::Mat m_old_frame;  ///< Previous frame
    cv::Mat m_current_frame;  ///< Current frame
    std::vector<cv::Point2f> m_p0;  ///< Previous feature points
    std::vector<cv::Point2f> m_p1;  ///< Current feature points
    std::vector<cv::Point2f> m_good_old;  ///< Good previous points
    std::vector<cv::Point2f> m_good_new;  ///< Good current points
    cv::Mat m_R;  ///< Rotation matrix (cumulative)
    cv::Mat m_t;  ///< Translation vector (cumulative)
    size_t m_frame_id;  ///< Current frame ID
    size_t m_n_features;  ///< Number of tracked features

    // Feature detection parameters
    cv::Ptr<cv::FastFeatureDetector> m_detector;  ///< FAST feature detector
    size_t m_lk_win_size;  ///< Lucas-Kanade window size
    cv::TermCriteria m_lk_criteria;  ///< Lucas-Kanade termination criteria
    size_t m_min_features;  ///< Minimum number of features to track

    // Scale estimation (optional, for datasets with ground truth)
    bool m_use_absolute_scale;  ///< Whether to use absolute scale from ground truth

    // TF
    std::shared_ptr<tf2_ros::TransformBroadcaster>
        m_tf_broadcaster;  ///< TF broadcaster

    // Diagnostics
    std::shared_ptr<diagnostic_updater::Updater>
        m_diagnostic_updater;  ///< Diagnostic updater

    // Publishers and subscribers
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr
        m_odom_pub;  ///< Odometry publisher
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr
        m_pose_pub;  ///< Pose publisher
    rclcpp::Publisher<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr
        m_pose_cov_pub;  ///< Pose with covariance publisher
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr
        m_debug_pub;  ///< Debug image publisher
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;  ///< Camera info subscriber
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr
        m_image_sub;  ///< Camera image subscriber

    // Frame tracking
    bool m_initialized;  ///< Whether the odometry is initialized
    std::string m_odom_frame_id;  ///< Odometry frame ID
    std::string m_base_frame_id;  ///< Base frame ID
};

}  // namespace clover2_odometry

