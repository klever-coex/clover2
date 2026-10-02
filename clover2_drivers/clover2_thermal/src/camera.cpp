#include <clover2_thermal/camera.hpp>
#include <clover2_thermal/thermal_frame.hpp>

#include <cv_bridge/cv_bridge.hpp>
#include <sensor_msgs/image_encodings.hpp>

#include <stdexcept>
#include <string>

namespace clover2_thermal {

camera::camera(const rclcpp::NodeOptions& options)
    : clover2_common::node("camera", options) {
    m_device = declare_parameter<std::string>("device", "/dev/thermal_camera");
    m_frame_id = declare_parameter<std::string>("frame_id", "camera_optical_frame");
    const bool publish_viz = declare_parameter<bool>("publish_viz", true);

    if (publish_viz) {
        m_viz_publisher = create_publisher<sensor_msgs::msg::Image>(
            "image_viz", rclcpp::SensorDataQoS());
    }
    m_temperature_publisher = create_publisher<sensor_msgs::msg::Image>(
        "temperature", rclcpp::SensorDataQoS());

    open_camera();
    m_capture_thread = std::thread(&camera::capture_loop, this);
}

camera::~camera() {
    m_running = false;
    m_capture_thread.join();
    m_capture.release();
}

void camera::open_camera() {
    if (!m_capture.open(m_device, cv::CAP_V4L2))
        throw std::runtime_error("Failed to open " + m_device);
    if (!m_capture.set(cv::CAP_PROP_CONVERT_RGB, 0.0))
        throw std::runtime_error("Failed to disable RGB conversion");
}

void camera::capture_loop() {
    try {
        cv::Mat frame;
        while (m_running &&
               rclcpp::ok(get_node_base_interface()->get_context())) {
            if (!m_capture.read(frame) || frame.empty()) {
                throw std::runtime_error("capture failed or returned empty frame");
            }

            const thermal_frame thermal(frame);

            std_msgs::msg::Header header;
            header.stamp = get_clock()->now();
            header.frame_id = m_frame_id;

            if (m_viz_publisher) {
                const cv::Mat grayscale = thermal.extract_grayscale();
                cv_bridge::CvImage viz_image(
                    header, sensor_msgs::image_encodings::MONO8, grayscale);
                m_viz_publisher->publish(*viz_image.toImageMsg());
            }

            const cv::Mat temperature = thermal.extract_kelvin_temperature();
            temperature -= 273.15f;  // Convert Kelvin to Celsius (REP 103)
            cv_bridge::CvImage image(
                header, sensor_msgs::image_encodings::TYPE_32FC1, temperature);
            m_temperature_publisher->publish(*image.toImageMsg());
        }
    } catch (const std::exception& error) {
        RCLCPP_ERROR(get_logger(), "Camera capture failed: %s", error.what());
    }
}

}  // namespace clover2_thermal

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_thermal::camera)
