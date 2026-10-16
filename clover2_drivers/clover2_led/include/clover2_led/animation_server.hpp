#pragma once

// clover2
#include <clover2_led/animation/base_animation.hpp>
#include <clover2_led/data/led_frame.hpp>
#include <clover2_led/device/base_device.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>

namespace clover2_led {

class animation_server {
public:
    explicit animation_server(int led_count,
                              std::shared_ptr<device::base_device> device,
                              rclcpp::Logger logger,
                              rclcpp::Clock::SharedPtr clock);

    void tick();

    void start(const animation::base_animation::Request& req);

    void cancel() noexcept;

    bool active() const;

private:
    int m_led_count;
    std::shared_ptr<device::base_device> m_device;
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;

    animation::base_animation::UniquePtr m_animation;
    data::led_frame m_last_frame;
};

}  // namespace clover2_led
