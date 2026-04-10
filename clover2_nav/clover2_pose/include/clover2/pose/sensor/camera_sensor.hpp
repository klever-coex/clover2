#pragma once

#include <clover2/pose/data/sensor_data.hpp>
#include <clover2/pose/sensor/base_sensor.hpp>

#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/camera_info.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <mutex>
#include <optional>

namespace clover2::pose::sensor {

class camera_sensor : public base_sensor {
public:
    RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(camera_sensor)

    explicit camera_sensor(creation_context& ctx, const std::string& subnode);
    ~camera_sensor();

    void start() override;
    void stop() override;

private:
    void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr msg);
    void camera_info_callback(
        const sensor_msgs::msg::CameraInfo::ConstSharedPtr msg);

    int32_t m_sensor_id = 0;
    sensor_callback m_callback;
    std::string m_image_topic;
    std::string m_camera_info_topic;

    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;
    rclcpp::Subscription<sensor_msgs::msg::CameraInfo>::SharedPtr
        m_camera_info_sub;

    std::mutex m_camera_info_mtx;
    std::optional<data::camera_intrinsics> m_intrinsics;
};

}  // namespace clover2::pose::sensor
