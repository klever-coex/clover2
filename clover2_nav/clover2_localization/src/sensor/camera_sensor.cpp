#include <clover2/localization/sensor/camera_sensor.hpp>

#include <pluginlib/class_list_macros.hpp>

#include <cv_bridge/cv_bridge.hpp>

namespace clover2::localization::sensor {

void camera_sensor::init(rclcpp::Node::SharedPtr node, sensor_callback callback,
                         const sensor_params& params) {
    m_node = node;
    m_sensor_id = params.sensor_id;
    m_callback = std::move(callback);
    m_image_topic = params.image_topic;
    m_camera_info_topic = params.camera_info_topic;
}

void camera_sensor::start() {
    m_image_sub = m_node->create_subscription<sensor_msgs::msg::Image>(
        m_image_topic, rclcpp::SensorDataQoS(),
        std::bind(&camera_sensor::image_callback, this, std::placeholders::_1));
    m_camera_info_sub =
        m_node->create_subscription<sensor_msgs::msg::CameraInfo>(
            m_camera_info_topic, rclcpp::SensorDataQoS(),
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

}  // namespace clover2::localization::sensor

PLUGINLIB_EXPORT_CLASS(clover2::localization::sensor::camera_sensor,
                       clover2::localization::sensor::base_sensor)
