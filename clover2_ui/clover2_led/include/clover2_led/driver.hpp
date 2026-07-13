#pragma once

// clover2
#include <clover2_common/node.hpp>
#include <clover2_led/data/driver_info.hpp>
#include <clover2_led/device/base_device.hpp>

// ROS2
#include <clover2_led_msgs/msg/led_frame.hpp>
#include <clover2_led_msgs/srv/get_current_frame.hpp>
#include <clover2_led_msgs/srv/get_driver_info.hpp>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_led {

class driver : public clover2_common::node {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(driver)

    explicit driver(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    ~driver() override;

private:
    void on_frame(const clover2_led_msgs::msg::LedFrame& msg);

    void handle_get_driver_info(
        const clover2_led_msgs::srv::GetDriverInfo::Request::SharedPtr req,
        clover2_led_msgs::srv::GetDriverInfo::Response::SharedPtr resp) const;

    void handle_get_current_frame(
        const clover2_led_msgs::srv::GetCurrentFrame::Request::SharedPtr req,
        clover2_led_msgs::srv::GetCurrentFrame::Response::SharedPtr resp);

    std::string m_plugin_class;
    data::led_frame m_last_frame{};
    pluginlib::ClassLoader<device::base_device> m_plugin_loader{
        "clover2_led", "clover2_led::device::base_device"};
    pluginlib::UniquePtr<device::base_device> m_device;
    rclcpp::Subscription<clover2_led_msgs::msg::LedFrame>::SharedPtr m_frame_sub;
    rclcpp::Service<clover2_led_msgs::srv::GetDriverInfo>::SharedPtr m_get_info_srv;
    rclcpp::Service<clover2_led_msgs::srv::GetCurrentFrame>::SharedPtr
        m_get_frame_srv;
};

}  // namespace clover2_led
