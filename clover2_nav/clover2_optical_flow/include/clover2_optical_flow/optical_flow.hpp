#pragma once

// STL includes
#include <mutex>

// ROS2 includes
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <image_geometry/pinhole_camera_model.hpp>
#include <rclcpp/rclcpp.hpp>

// Clover2 includes
#include <clover2_common/lifecycle_node.hpp>

// TF2 includes
#include <tf2/exceptions.h>
#include <tf2/convert.h>
#include <tf2/utils.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

// OpenCV includes
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.hpp>

// Msgs includes
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/vector3_stamped.hpp>
#include <mavros_msgs/msg/optical_flow_rad.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>

namespace clover2_optical_flow {

/**
 * @class optical_flow
 * @brief Lifecycle node for calculating optical flow using phase correlation.
 *
 * This node subscribes to camera images and camera info, calculates optical flow
 * using phase correlation, and publishes flow data in MAVROS format.
 */
class optical_flow : public clover2_common::lifecycle_node {
public:
    using SharedPtr = std::shared_ptr<optical_flow>;
    using CallbackReturn =
        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

    /**
     * @brief Construct a new optical_flow node.
     * @param options Node options for ROS2 configuration
     */
    explicit optical_flow(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    CallbackReturn on_configure(const rclcpp_lifecycle::State& /* state */);
    CallbackReturn on_activate(const rclcpp_lifecycle::State& /* state */);
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& /* state */);
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& /* state */);
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& /* state */);

    /**
     * @brief Callback for image topic subscription.
     * @param msg Incoming camera image
     */
    void flow_callback(const sensor_msgs::msg::Image::ConstSharedPtr& msg);

    /**
     * @brief Callback for camera info topic subscription.
     * @param msg Incoming camera info message
     */
    void camera_info_callback(
        const sensor_msgs::msg::CameraInfo::ConstSharedPtr& msg);

    /**
     * @brief Draw flow visualization on frame.
     * @param frame Image frame to draw on
     * @param x Flow x component
     * @param y Flow y component
     * @param quality Flow quality
     */
    void draw_flow(cv::Mat& frame, double x, double y, double quality) const;

    /**
     * @brief Calculate flow gyro from TF transforms.
     * @param frame_id Frame ID for transform lookup
     * @param prev Previous timestamp
     * @param curr Current timestamp
     * @return Flow gyro vector in camera frame
     */
    geometry_msgs::msg::Vector3Stamped calc_flow_gyro(
        const std::string& frame_id, const rclcpp::Time& prev,
        const rclcpp::Time& curr);

private:
    // Parameters
    int m_roi_px;                      ///< ROI size in pixels
    bool m_calc_flow_gyro;             ///< Calculate flow gyro from TF
    float m_flow_gyro_default;         ///< Default flow gyro value
    std::string m_fcu_frame_id;        ///< FCU frame ID
    std::string m_local_frame_id;      ///< Local frame ID

    // Camera parameters
    std::mutex m_camera_info_mtx;      ///< Mutex for thread-safe camera info access
    image_geometry::PinholeCameraModel m_camera_model;  ///< Camera model

    // State
    rclcpp::Time m_prev_stamp;         ///< Previous frame timestamp
    rclcpp::Time m_last_vpe_time;      ///< Last VPE message timestamp
    cv::Rect m_roi;                    ///< Region of interest
    cv::Mat m_hann;                    ///< Hanning window for phase correlation
    cv::Mat m_prev;                    ///< Previous frame
    cv::Mat m_curr;                    ///< Current frame

    // TF
    std::unique_ptr<tf2_ros::Buffer> m_tf_buffer;           ///< TF buffer
    std::unique_ptr<tf2_ros::TransformListener> m_tf_listener;  ///< TF listener

    // Diagnostics
    std::shared_ptr<diagnostic_updater::Updater> m_diagnostic_updater;

    // Publishers and subscribers
    rclcpp::Publisher<mavros_msgs::msg::OpticalFlowRad>::SharedPtr
        m_flow_pub;  ///< Flow publisher
    rclcpp::Publisher<geometry_msgs::msg::Vector3Stamped>::SharedPtr
        m_shift_pub;  ///< Shift publisher
    rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr
        m_debug_pub;  ///< Debug image publisher
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;  ///< Camera info subscriber

    // Image subscriber
    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr
        m_image_sub;  ///< Camera image subscriber
};

}  // namespace clover2_optical_flow
