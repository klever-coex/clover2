#pragma once

// clover2
#include <clover2_common/node.hpp>
#include <clover2_led/data/driver_info.hpp>
#include <clover2_led/device/base_device.hpp>

// ROS2
#include <clover2_led/msg/led_frame.hpp>
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>

namespace clover2_led {

class driver : public clover2_common::node {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(driver)

    explicit driver(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    ~driver() override;

private:
    void on_frame(const clover2_led::msg::LedFrame& msg);

    data::driver_info m_info{};
    pluginlib::ClassLoader<device::base_device> m_plugin_loader{
        "clover2_led", "clover2_led::device::base_device"};
    std::unique_ptr<device::base_device> m_device;
    rclcpp::Subscription<clover2_led::msg::LedFrame>::SharedPtr m_frame_sub;
};

}  // namespace clover2_led
