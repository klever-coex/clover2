#pragma once

// STL
#include <mutex>

// ROS2 includes
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// Clover2 includes
#include <clover2/aruco/diagnostics/map_task.hpp>
#include <clover2/map/client.hpp>
#include <clover2_common/lifecycle_node.hpp>

// TF2 includes
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/transform_listener.h>

// Msgs includes
#include <clover2_pose_msgs/msg/marker_array.hpp>
#include <geometry_msgs/msg/pose_array.hpp>
#include <geometry_msgs/msg/pose_with_covariance_stamped.hpp>

namespace clover2::aruco {

class tracker : public clover2_common::lifecycle_node {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    using SharedPtr = std::shared_ptr<tracker>;
    using CallbackReturn = rclcpp_lifecycle::node_interfaces::
        LifecycleNodeInterface::CallbackReturn;
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    explicit tracker(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    virtual ~tracker();

    CallbackReturn on_configure(const rclcpp_lifecycle::State& state);
    CallbackReturn on_activate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& state);
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& state);
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& state);

private:
    void markers_callback(
        const clover2_pose_msgs::msg::MarkerArray::SharedPtr msg);

    void publish_tf(const std_msgs::msg::Header& header,
                    Eigen::Isometry3d pose);

    void produce_markers_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& stat);
    void produce_pose_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& stat);
    void produce_pose_hz_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& stat);

    // Camera parameters
    std::string m_frame_id;

    // Publish parameters
    double m_xy_variation{0.4}, m_z_variation{0.4};

    // Detection parameters
    std::shared_ptr<clover2::map::client> m_map_client;

    // TF
    bool m_tf_send{false};
    std::string m_child_frame_id;
    std::shared_ptr<tf2_ros::TransformBroadcaster> m_tf_broadcaster;
    std::shared_ptr<tf2_ros::Buffer> m_tf_buffer;
    std::shared_ptr<tf2_ros::TransformListener> m_tf_listener;

    // Diagnostics
    mutable std::mutex m_diagnostics_mtx;
    rclcpp::Time m_last_markers_stamp;
    rclcpp::Time m_last_pose_publish_stamp;
    size_t m_processed_marker_arrays{0};
    size_t m_last_marker_count{0};
    bool m_last_tf_ok{true};
    double m_min_pose_hz{25.0};
    rclcpp::Time m_last_pose_hz_stamp;
    size_t m_last_pose_processed_marker_arrays{0};
    double m_pose_hz{0.0};

    // Publishers and subscribers
    rclcpp::CallbackGroup::SharedPtr m_callback_group;
    rclcpp::Subscription<clover2_pose_msgs::msg::MarkerArray>::SharedPtr
        m_markers_sub;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr m_pose_pub;
    rclcpp::Publisher<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr
        m_pose_cov_pub;
    rclcpp::Publisher<geometry_msgs::msg::PoseArray>::SharedPtr
        m_poses_debug_pub;
};

}  // namespace clover2::aruco
