#include <clover2_thermal/camera.hpp>

#include <cv_bridge/cv_bridge.hpp>
#include <sensor_msgs/image_encodings.hpp>

#include <stdexcept>
#include <string>

namespace clover2_thermal {

camera::camera(const rclcpp::NodeOptions& options)
    : clover2_common::node("camera", options) {
    if (!m_capture.open("/dev/video1", cv::CAP_V4L2)) {
        throw std::runtime_error("Failed to open /dev/video1");
    }
    if (!m_capture.set(cv::CAP_PROP_CONVERT_RGB, 0.0)) {
        throw std::runtime_error("Failed to disable RGB conversion");
    }

    m_viz_publisher = create_publisher<sensor_msgs::msg::Image>(
        "image_viz", rclcpp::SensorDataQoS());
    m_temperature_publisher = create_publisher<sensor_msgs::msg::Image>(
        "temperature", rclcpp::SensorDataQoS());
    m_capture_thread = std::thread(&camera::capture_loop, this);
}

camera::~camera() {
    m_running = false;
    if (m_capture_thread.joinable()) m_capture_thread.join();
    m_capture.release();
}

void camera::capture_loop() {
    try {
        cv::Mat frame;
        while (m_running &&
               rclcpp::ok(get_node_base_interface()->get_context())) {
            if (!m_capture.read(frame) || frame.empty()) {
                RCLCPP_ERROR(get_logger(), "Failed to read a camera frame");
                break;
            }

            if (frame.type() != CV_8UC2 || frame.rows % 2 != 0) {
                RCLCPP_ERROR(get_logger(), "Unexpected frame format");
                break;
            }

            int h = frame.rows / 2;
            int w = frame.cols;

            cv::Mat upper = frame(cv::Rect(0, 0, w, h));
            cv::Mat grayscale;
            cv::extractChannel(upper, grayscale, 0);
            
            cv_bridge::CvImage viz_image;
            viz_image.header.stamp = get_clock()->now();
            viz_image.header.frame_id = "camera_optical_frame";
            viz_image.encoding = sensor_msgs::image_encodings::MONO8;
            viz_image.image = grayscale;
            m_viz_publisher->publish(*viz_image.toImageMsg());

            cv::Mat lower = frame(cv::Rect(0, h, w, h));
            cv::Mat raw16(h, w, CV_16UC1, lower.data, lower.step);
            cv::Mat temperature;
            raw16.convertTo(temperature, CV_32FC1, 1.0 / 64.0, -273.15);

            cv_bridge::CvImage image;
            image.header.stamp = get_clock()->now();
            image.header.frame_id = "camera_optical_frame";
            image.encoding = sensor_msgs::image_encodings::TYPE_32FC1;
            image.image = temperature;
            m_temperature_publisher->publish(*image.toImageMsg());
        }
    } catch (const std::exception& error) {
        RCLCPP_ERROR(get_logger(), "Camera capture failed: %s", error.what());
    }
}

}  // namespace clover2_thermal

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_thermal::camera)
