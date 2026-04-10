// clover2
#include <clover2/aruco/optimizer/fabric.hpp>
#include <clover2/aruco/optimizer/simple_mean.hpp>
#include <clover2/aruco/tracker.hpp>
#include <clover2/aruco/util/type_support.hpp>

// Tf2
#include <tf2/LinearMath/Quaternion.hpp>
#include <tf2/LinearMath/Transform.hpp>
#include <tf2_eigen/tf2_eigen.hpp>

// ROS2 msgs
#include <lifecycle_msgs/msg/state.hpp>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <tf2_msgs/msg/tf_message.hpp>

// STL
#include <chrono>

namespace clover2::aruco {

tracker::tracker(const rclcpp::NodeOptions& options)
    : clover2::common::lifecycle_node("tracker", options) {
    m_parameter_watcher =
        std::make_shared<clover2::common::parameter_watcher>(*this);

    m_parameter_watcher->declare_and_watch_parameter<std::string>(
        "tracking", "base_link",
        [this](const rclcpp::Parameter& p) { m_tracking_id = p.as_string(); },
        "Tracking result");

    m_parameter_watcher->declare_and_watch_parameter<std::string>(
        "optimizer", "simple_mean",
        [this](const rclcpp::Parameter& p) {
            auto optimizer_name = p.as_string();
            auto available_optimizers =
                optimizer::fabric::instance().list_optimizers();

            if (std::find(available_optimizers.begin(),
                          available_optimizers.end(),
                          optimizer_name) != available_optimizers.end()) {
                m_optimizer_type = optimizer_name;
            } else {
                throw std::runtime_error("Unknown optimizer type: " +
                                         optimizer_name);
            }
        },
        "Select optimizer to use for pose estimation");

    register_on_configure(
        std::bind(&tracker::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&tracker::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&tracker::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&tracker::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&tracker::on_shutdown, this, std::placeholders::_1));
}

tracker::~tracker() {}

tracker::CallbackReturn tracker::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_tf_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(*this);
    m_tf_buffer = std::make_shared<tf2_ros::Buffer>(this->get_clock());
    m_tf_listener = std::make_shared<tf2_ros::TransformListener>(*m_tf_buffer);

    m_map_client =
        std::make_shared<clover2::map_server::map_client>(shared_from_this());

    m_pose_pub = create_publisher<geometry_msgs::msg::PoseStamped>(
        "~/pose", rclcpp::SystemDefaultsQoS());

    m_pose_cov_pub =
        create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>(
            "~/pose_cov", rclcpp::SystemDefaultsQoS());

    m_poses_debug_pub = create_publisher<geometry_msgs::msg::PoseArray>(
        "~/poses_debug", rclcpp::SystemDefaultsQoS());

    m_markers_sub = create_subscription<clover2_aruco_msgs::msg::MarkerArray>(
        "~/markers", rclcpp::SensorDataQoS(),
        std::bind(&tracker::markers_callback, this, std::placeholders::_1));

    try {
        initialize_optimizer();
    } catch (const std::runtime_error& e) {
        RCLCPP_ERROR(get_logger(), "Failed to initialize optimizer: %s",
                     e.what());
        return tracker::CallbackReturn::FAILURE;
    }

    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    m_markers_sub.reset();
    m_pose_pub.reset();
    m_pose_cov_pub.reset();
    m_poses_debug_pub.reset();

    m_optimizer.reset();
    m_map_client.reset();

    m_tf_listener.reset();
    m_tf_buffer.reset();
    m_tf_broadcaster.reset();

    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    return tracker::CallbackReturn::SUCCESS;
}

tracker::CallbackReturn tracker::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    return tracker::CallbackReturn::SUCCESS;
}

void tracker::markers_callback(
    const clover2_aruco_msgs::msg::MarkerArray::SharedPtr msg) {
    std::lock_guard<clover2::map_server::map_client> map_guard(*m_map_client);

    if (msg->markers.size() == 0) {
        return;
    }

    // transform between tracking frame and camera frame
    Eigen::Affine3d camera_transform;
    try {
        auto camera_transform_msg = m_tf_buffer->lookupTransform(
            m_tracking_id, msg->header.frame_id, tf2::TimePointZero);

        camera_transform = tf2::transformToEigen(camera_transform_msg);
    } catch (const tf2::TransformException& ex) {
        RCLCPP_ERROR(get_logger(), "Unable got transform %s to %s: %s",
                     m_tracking_id.c_str(), msg->header.frame_id.c_str(),
                     ex.what());
        return;
    }

    // debug poses of camera from each marker
    geometry_msgs::msg::PoseArray poses_debug;
    poses_debug.header.stamp = msg->header.stamp;
    poses_debug.header.frame_id = m_map_client->get_map_id();
    poses_debug.poses.reserve(msg->markers.size());

    std::vector<optimizer::marker> measurements;
    measurements.reserve(msg->markers.size());

    Eigen::Affine3d marker_pose;

    for (const auto& marker : msg->markers) {
        tf2::fromMsg(marker.pose.pose, marker_pose);

        Eigen::Affine3d camera_in_map =
            m_map_client->get_transform(marker.id) * marker_pose.inverse();
        Eigen::Affine3d drone_in_map = camera_in_map * camera_transform;

        // fill marker position with covariance
        optimizer::marker measurement;
        measurement.id = marker.id;
        measurement.transform = drone_in_map;
        clover2::aruco::util::cov_ros_to_eigen(marker.pose.covariance,
                                               measurement.cov);

        // add measurement for optimizer
        measurements.push_back(measurement);

        // fill debug msg
        poses_debug.poses.push_back(tf2::toMsg(drone_in_map));
    }

    if (!m_optimizer) {
        RCLCPP_ERROR(get_logger(), "Optimizer is not initialized");
        return;
    }

    auto timestamp =
        std::chrono::nanoseconds(rclcpp::Time(msg->header.stamp).nanoseconds());
    m_optimizer->push_measurements(msg->header.frame_id, timestamp,
                                   measurements);

    m_optimizer->optimize();

    // publish tracker id poses form each marker
    if (m_poses_debug_pub->get_subscription_count() != 0) {
        m_poses_debug_pub->publish(poses_debug);
    }
}

void tracker::initialize_optimizer() {
    optimizer::context ctx;
    ctx.node_base = get_node_base_interface();
    ctx.node_logging = get_node_logging_interface();
    ctx.node_parameters = get_node_parameters_interface();
    ctx.node_timers = get_node_timers_interface();
    ctx.map_client = m_map_client;

    RCLCPP_INFO(get_logger(), "Using %s optimizer", m_optimizer_type.c_str());
    m_optimizer = optimizer::fabric::instance().create(m_optimizer_type, ctx);

    if (!m_optimizer) {
        throw std::runtime_error("Failed to create optimizer");
    }

    m_optimizer->set_data_ready_callback(
        [this](const optimizer::marker& pose,
               std::chrono::nanoseconds timestamp) {
            publish_pose(pose, timestamp);
        });
}

void tracker::publish_pose(const optimizer::marker& pose,
                           std::chrono::nanoseconds timestamp) {
    geometry_msgs::msg::PoseStamped estimated_pose;
    estimated_pose.header.stamp =
        rclcpp::Time(static_cast<int64_t>(timestamp.count()));
    estimated_pose.header.frame_id = m_map_client->get_map_id();
    estimated_pose.pose = tf2::toMsg(pose.transform);

    geometry_msgs::msg::PoseWithCovarianceStamped estimated_pose_cov;
    estimated_pose_cov.header = estimated_pose.header;
    estimated_pose_cov.pose.pose = estimated_pose.pose;
    clover2::aruco::util::cov_eigen_to_ros(pose.cov,
                                           estimated_pose_cov.pose.covariance);

    m_pose_pub->publish(estimated_pose);
    m_pose_cov_pub->publish(estimated_pose_cov);
}

}  // namespace clover2::aruco

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::aruco::tracker)
