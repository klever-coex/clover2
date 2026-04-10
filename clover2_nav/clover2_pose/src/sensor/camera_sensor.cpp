#include "clover2/pose/sensor/base_sensor.hpp"
#include <clover2/pose/sensor/camera_sensor.hpp>

#include <pluginlib/class_list_macros.hpp>

#include <cv_bridge/cv_bridge.hpp>

namespace clover2::pose::sensor {

camera_sensor::camera_sensor(creation_context& ctx, const std::string& subnode) :
    base_sensor(ctx, subnode) {

    declare_and_watch_parameter(get_node()->get_name() + "");
}

void camera_sensor::start() {
    m_image_sub = get_node()->create_subscription<sensor_msgs::msg::Image>(
        "~/image_raw", rclcpp::SensorDataQoS(),
        std::bind(&camera_sensor::image_callback, this, std::placeholders::_1));
    m_camera_info_sub =
        get_node()->create_subscription<sensor_msgs::msg::CameraInfo>(
            "~/camera_info", rclcpp::SensorDataQoS(),
            std::bind(&camera_sensor::camera_info_callback, this,
                      std::placeholders::_1));
}

void camera_sensor::stop() {
    m_image_sub.reset();
    m_camera_info_sub.reset();
}

void camera_sensor::image_callback(
    const sensor_msgs::msg::Image::ConstSharedPtr msg) {
    cv::Mat image;
    try {
        image = cv_bridge::toCvShare(msg, "bgr8")->image;
    } catch (const cv_bridge::Exception&) {
        return;
    }

    data::sensor_data sd;
    sd.timestamp =
        rclcpp::Time(msg->header.stamp).seconds();
    sd.sensor_id = m_sensor_id;
    sd.image = image;

    {
        std::lock_guard<std::mutex> lock(m_camera_info_mtx);
        if (m_intrinsics.has_value()) {
            sd.intrinsics = m_intrinsics;
        }
    }

    if (m_callback) {
        m_callback(sd);
    }
}

void camera_sensor::camera_info_callback(
    const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg) {
    if (msg->height == 0 || msg->width == 0) {
        return;
    }

    data::camera_intrinsics intrinsics;
    intrinsics.K = cv::Mat(3, 3, CV_64F);
    for (int i = 0; i < 9; ++i) {
        intrinsics.K.at<double>(i / 3, i % 3) = msg->k[i];
    }
    intrinsics.dist_coeffs = cv::Mat(msg->d);
    if (intrinsics.dist_coeffs.rows == 1) {
        intrinsics.dist_coeffs = intrinsics.dist_coeffs.t();
    }

    std::lock_guard<std::mutex> lock(m_camera_info_mtx);
    m_intrinsics = intrinsics;
}

}  // namespace clover2::pose::sensor

PLUGINLIB_EXPORT_CLASS(clover2::pose::sensor::camera_sensor,
                       clover2::pose::sensor::base_sensor)
