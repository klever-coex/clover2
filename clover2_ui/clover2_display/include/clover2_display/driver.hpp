#pragma once

// clover2
#include <clover2_common/node.hpp>
#include <clover2_display/device/base_device.hpp>

// ROS 2
#include <clover2_display_msgs/srv/get_driver_info.hpp>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_display {

class driver : public clover2_common::node {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(driver)

    explicit driver(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~driver() override;

private:
    void on_image(const sensor_msgs::msg::Image& msg);

    void handle_get_driver_info(
        const clover2_display_msgs::srv::GetDriverInfo::Request::SharedPtr req,
        clover2_display_msgs::srv::GetDriverInfo::Response::SharedPtr resp)
        const;

    std::string m_plugin_class;
    pluginlib::ClassLoader<device::base_device> m_plugin_loader{
        "clover2_display", "clover2_display::device::base_device"};
    std::shared_ptr<device::base_device> m_device;

    rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr m_image_sub;
    rclcpp::Service<clover2_display_msgs::srv::GetDriverInfo>::SharedPtr
        m_get_info_srv;
};

}  // namespace clover2_display
