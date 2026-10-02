#include <clover2_thermal/camera.hpp>
#include <clover2_thermal/thermal_frame.hpp>

#include <cv_bridge/cv_bridge.hpp>
#include <linux/videodev2.h>
#include <sensor_msgs/image_encodings.hpp>
#include <sys/ioctl.h>

#include <cstdint>
#include <fcntl.h>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace clover2_thermal {

camera::camera(const rclcpp::NodeOptions& options)
    : clover2_common::node("camera", options) {
    m_device = declare_parameter<std::string>("device", "/dev/thermal_camera");
    m_frame_id =
        declare_parameter<std::string>("frame_id", "camera_optical_frame");
    const bool publish_viz = declare_parameter<bool>("publish_viz", true);

    if (publish_viz) {
        m_viz_publisher = create_publisher<sensor_msgs::msg::Image>(
            "image_viz", rclcpp::SensorDataQoS());
    }
    m_temperature_publisher = create_publisher<sensor_msgs::msg::Image>(
        "temperature", rclcpp::SensorDataQoS());

    open_camera();
    log_camera_info();
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

void camera::log_camera_info() {
    std::string name = "unknown";
    std::string location = "unknown";
    const int fd = ::open(m_device.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd >= 0) {
        v4l2_capability capability{};
        if (::ioctl(fd, VIDIOC_QUERYCAP, &capability) == 0) {
            name = reinterpret_cast<const char*>(capability.card);
            location = reinterpret_cast<const char*>(capability.bus_info);
        } else {
            RCLCPP_WARN(get_logger(),
                        "Failed to query device information for %s",
                        m_device.c_str());
        }
        ::close(fd);
    } else {
        RCLCPP_WARN(get_logger(), "Failed to open %s for device information",
                    m_device.c_str());
    }

    const auto fourcc =
        static_cast<std::uint32_t>(m_capture.get(cv::CAP_PROP_FOURCC));
    const char pixel_format[] = {
        static_cast<char>(fourcc & 0xff),
        static_cast<char>((fourcc >> 8) & 0xff),
        static_cast<char>((fourcc >> 16) & 0xff),
        static_cast<char>((fourcc >> 24) & 0xff),
        '\0',
    };
    RCLCPP_INFO(get_logger(), "Device: %s, name: %s, location: %s",
                m_device.c_str(), name.c_str(), location.c_str());
    RCLCPP_INFO(get_logger(), "Pixel format: %s, resolution: %dx%d",
                pixel_format,
                static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_WIDTH)),
                static_cast<int>(m_capture.get(cv::CAP_PROP_FRAME_HEIGHT)));
}

void camera::capture_loop() {
    try {
        cv::Mat frame;
        while (m_running &&
               rclcpp::ok(get_node_base_interface()->get_context())) {
            if (!m_capture.read(frame) || frame.empty()) {
                throw std::runtime_error(
                    "capture failed or returned empty frame");
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
