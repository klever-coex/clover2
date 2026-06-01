// clover2
#include <clover2/optical_flow/optical_flow.hpp>

// ROS2
#include <tf2/LinearMath/Quaternion.hpp>

// msgs
#include <lifecycle_msgs/msg/state.hpp>
#include <sensor_msgs/image_encodings.hpp>

// STL
#include <cmath>
#include <memory>
#include <vector>

namespace clover2::optical_flow {

optical_flow::optical_flow(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("optical_flow", options)
    , m_fcu_frame_id("base_link")
    , m_local_frame_id("map")
    , m_prev_stamp(rclcpp::Time(0))
    , m_last_vpe_time(rclcpp::Time(0)) {

    m_diagnostic_updater = get_diagnostic_updater();

    // Declare parameters
    declare_and_watch_parameter<int>(
        "roi", 256,
        [this](const rclcpp::Parameter& p) { m_roi_px = p.as_int(); },
        "ROI size in pixels");

    declare_and_watch_parameter<bool>(
        "calc_flow_gyro", false,
        [this](const rclcpp::Parameter& p) { m_calc_flow_gyro = p.as_bool(); },
        "Calculate flow gyro from TF transforms");

    declare_and_watch_parameter<double>(
        "flow_gyro_default", NAN,
        [this](const rclcpp::Parameter& p) {
            m_flow_gyro_default = static_cast<float>(p.as_double());
        },
        "Default flow gyro value");

    declare_parameter("mavros.local_position.tf.frame_id", m_local_frame_id);
    declare_parameter("mavros.local_position.tf.child_frame_id",
                      m_fcu_frame_id);

    // Register lifecycle callbacks
    register_on_configure(
        std::bind(&optical_flow::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&optical_flow::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&optical_flow::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&optical_flow::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&optical_flow::on_shutdown, this, std::placeholders::_1));
}

optical_flow::CallbackReturn optical_flow::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    // Get frame IDs from parameters
    m_local_frame_id =
        get_parameter("mavros.local_position.tf.frame_id").as_string();
    m_fcu_frame_id =
        get_parameter("mavros.local_position.tf.child_frame_id").as_string();

    // Initialize TF buffer and listener
    m_tf_buffer = std::make_unique<tf2_ros::Buffer>(get_clock());
    m_tf_listener = std::make_unique<tf2_ros::TransformListener>(
        *m_tf_buffer, shared_from_this());

    RCLCPP_INFO(get_logger(), "Optical Flow configured");

    return CallbackReturn::SUCCESS;
}

optical_flow::CallbackReturn optical_flow::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    // Create publishers
    m_flow_pub = this->create_publisher<mavros_msgs::msg::OpticalFlowRad>(
        "mavros/px4flow/raw/send", rclcpp::SystemDefaultsQoS());
    m_debug_pub = this->create_publisher<sensor_msgs::msg::Image>(
        "~/debug", rclcpp::SystemDefaultsQoS());

    // Create subscribers
    m_camera_info_sub = this->create_subscription<sensor_msgs::msg::CameraInfo>(
        "~/input/camera_info", rclcpp::SensorDataQoS(),
        std::bind(&optical_flow::camera_info_callback, this,
                  std::placeholders::_1));

    m_image_sub = this->create_subscription<sensor_msgs::msg::Image>(
        "~/input/image_raw", rclcpp::SensorDataQoS(),
        std::bind(&optical_flow::flow_callback, this, std::placeholders::_1));

    RCLCPP_INFO(get_logger(), "Optical Flow activated");

    return CallbackReturn::SUCCESS;
}

optical_flow::CallbackReturn optical_flow::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    // Reset publishers and subscribers
    m_flow_pub.reset();
    m_debug_pub.reset();
    m_camera_info_sub.reset();
    m_image_sub.reset();

    return CallbackReturn::SUCCESS;
}

optical_flow::CallbackReturn optical_flow::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    // Cleanup TF
    m_tf_buffer.reset();
    m_tf_listener.reset();

    // Clear state
    m_prev = cv::Mat();
    m_curr = cv::Mat();
    m_roi = cv::Rect();

    return CallbackReturn::SUCCESS;
}

optical_flow::CallbackReturn optical_flow::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& /* state */) {
    // Reset all resources
    m_flow_pub.reset();
    m_debug_pub.reset();
    m_image_sub.reset();
    m_camera_info_sub.reset();
    m_tf_buffer.reset();
    m_tf_listener.reset();

    return CallbackReturn::SUCCESS;
}

void optical_flow::camera_info_callback(
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr& msg) {
    std::lock_guard<std::mutex> guard(m_camera_info_mtx);

    // validate camera info
    if (msg->height == 0 || msg->width == 0 || msg->d.size() == 0) {
        return;
    }

    m_camera_model.fromCameraInfo(msg);
}

void optical_flow::draw_flow(cv::Mat& frame, double x, double y,
                             double quality) const {
    double brightness = (1 - quality) * 25;
    cv::Scalar color(brightness, brightness, brightness);
    double radius = std::sqrt(x * x + y * y);

    cv::Point center(frame.cols >> 1, frame.rows >> 1);
    cv::circle(frame, center, static_cast<int>(radius * 5), color, 3,
               cv::LINE_AA);
    cv::line(frame, center,
             cv::Point(center.x + static_cast<int>(x * 5),
                       center.y + static_cast<int>(y * 5)),
             color, 3, cv::LINE_AA);
}

void optical_flow::flow_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr& msg) {
    std::lock_guard<std::mutex> camera_info_guard(m_camera_info_mtx);

    if (!m_camera_model.initialized()) {
        RCLCPP_ERROR(get_logger(), "Camera info not initialized");
        return;
    }

    auto img = cv_bridge::toCvShare(msg, "mono8")->image;

    if (m_roi_px > 0) {
        m_roi = cv::Rect((msg->width / 2 - m_roi_px / 2),
                         (msg->height / 2 - m_roi_px / 2), m_roi_px, m_roi_px);
    }

    if (m_roi.width != 0) {
        img = img(m_roi);
    }

    img.convertTo(m_curr, CV_32F);

    rclcpp::Time current_stamp(msg->header.stamp);
    if (m_prev.empty() || (current_stamp - m_prev_stamp).seconds() >
                              0.1) {  // outdated previous frame
        m_prev = m_curr.clone();
        m_prev_stamp = current_stamp;
        cv::createHanningWindow(m_hann, m_curr.size(), CV_32F);

        return;
    }

    mavros_msgs::msg::OpticalFlowRad flow_msg;
    flow_msg.header.stamp = msg->header.stamp;
    flow_msg.time_delta_distance_us = 0;
    flow_msg.distance = -1;
    flow_msg.temperature = 0;

    double response;
    cv::Point2d phase_shift =
        cv::phaseCorrelate(m_prev, m_curr, m_hann, &response);

    // Undistort flow in pixels
    cv::Point2d image_center = cv::Point2d(msg->width, msg->height) / 2.0;
    cv::Point2d shift = phase_shift + image_center;

    std::vector<cv::Point2d> points_dist = {shift};
    std::vector<cv::Point2d> points_undist(1);

    auto camera_matrix = m_camera_model.fullIntrinsicMatrix();
    auto dist_coeffs = m_camera_model.distortionCoeffs();
    cv::undistortPoints(points_dist, points_undist, camera_matrix, dist_coeffs,
                        cv::noArray(), camera_matrix);
    points_undist[0] -= image_center;

    // Calculate flow in radians
    double focal_length_x = camera_matrix(0, 0);
    double focal_length_y = camera_matrix(1, 1);
    double flow_x = atan2(points_undist[0].x, focal_length_x);
    double flow_y = atan2(points_undist[0].y, focal_length_y);

    // Convert to FCU frame
    geometry_msgs::msg::Vector3Stamped flow_camera, flow_fcu;
    flow_camera.header.frame_id = msg->header.frame_id;
    flow_camera.header.stamp = msg->header.stamp;
    flow_camera.vector.x = flow_y;
    flow_camera.vector.y = -flow_x;

    try {
        m_tf_buffer->transform(flow_camera, flow_fcu, m_fcu_frame_id);
    } catch (const tf2::TransformException& e) {
        return;
    }

    // Calculate integration time
    rclcpp::Duration integration_time = current_stamp - m_prev_stamp;
    uint32_t integration_time_us =
        static_cast<uint32_t>(integration_time.seconds() * 1.0e6);

    // Calculate flow gyro
    flow_msg.integrated_xgyro = m_flow_gyro_default;
    flow_msg.integrated_ygyro = m_flow_gyro_default;
    flow_msg.integrated_zgyro = m_flow_gyro_default;

    if (m_calc_flow_gyro) {
        try {
            auto flow_gyro_camera = calc_flow_gyro(msg->header.frame_id,
                                                   m_prev_stamp, current_stamp);
            geometry_msgs::msg::Vector3Stamped flow_gyro_fcu;
            m_tf_buffer->transform(flow_gyro_camera, flow_gyro_fcu,
                                   m_fcu_frame_id);
            flow_msg.integrated_xgyro = flow_gyro_fcu.vector.x;
            flow_msg.integrated_ygyro = flow_gyro_fcu.vector.y;
            flow_msg.integrated_zgyro = flow_gyro_fcu.vector.z;
        } catch (const tf2::TransformException& e) {
            // Transform not available, keep NANs in flow gyro
        }
    }

    // Publish flow in fcu frame
    flow_msg.integration_time_us = integration_time_us;
    flow_msg.integrated_x = flow_fcu.vector.x;
    flow_msg.integrated_y = flow_fcu.vector.y;
    flow_msg.quality = static_cast<uint8_t>(response * 255);
    m_flow_pub->publish(flow_msg);

    m_prev = m_curr.clone();
    m_prev_stamp = current_stamp;

    // Publish debug image
    if (m_debug_pub->get_subscription_count() > 0) {
        // publish debug image
        draw_flow(img, phase_shift.x, phase_shift.y, response);
        cv_bridge::CvImage out_msg;
        out_msg.header.frame_id = msg->header.frame_id;
        out_msg.header.stamp = msg->header.stamp;
        out_msg.encoding = sensor_msgs::image_encodings::MONO8;
        out_msg.image = img;
        m_debug_pub->publish(*out_msg.toImageMsg());
    }
}

geometry_msgs::msg::Vector3Stamped optical_flow::calc_flow_gyro(
    const std::string& frame_id, const rclcpp::Time& prev,
    const rclcpp::Time& curr) {
    geometry_msgs::msg::TransformStamped prev_tf, curr_tf;
    try {
        prev_tf =
            m_tf_buffer->lookupTransform(frame_id, m_local_frame_id, prev);
        curr_tf =
            m_tf_buffer->lookupTransform(frame_id, m_local_frame_id, curr,
                                         rclcpp::Duration::from_seconds(0.1));
    } catch (const tf2::TransformException& e) {
        throw;
    }

    tf2::Quaternion prev_rot, curr_rot;
    tf2::fromMsg(prev_tf.transform.rotation, prev_rot);
    tf2::fromMsg(curr_tf.transform.rotation, curr_rot);

    geometry_msgs::msg::Vector3Stamped flow;
    flow.header.frame_id = frame_id;
    flow.header.stamp = curr;

    auto diff = ((curr_rot - prev_rot) * prev_rot.inverse()) * 2.0f;
    flow.vector.x = -diff.x();
    flow.vector.y = -diff.y();
    flow.vector.z = -diff.z();

    return flow;
}

}  // namespace clover2::optical_flow

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::optical_flow::optical_flow)
