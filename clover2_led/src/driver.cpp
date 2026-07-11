#include <clover2_led/driver.hpp>

// clover2
#include <clover2_led/data/led_frame.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_led {

driver::driver(const rclcpp::NodeOptions& options)
    : clover2_common::node("clover2_led_driver", options) {
    enable_diagnostic_updater();

    declare_and_watch_parameter<std::string>(
        "device_plugin", "clover2_led::plugin::device::WS2812",
        [this](const rclcpp::Parameter& p) {
            if (m_device) {
                m_device->set_brightness(static_cast<float>(p.as_double()));
            }
        },
        "Led driver plugin selection");

    declare_and_watch_parameter<double>(
        "brightness", 1.0,
        [this](const rclcpp::Parameter& p) {
            if (m_device) {
                m_device->set_brightness(static_cast<float>(p.as_double()));
            }
        },
        "Master brightness [0.0, 1.0]");

    declare_and_watch_parameter<int64_t>(
        "led_count", 0,
        [this](const rclcpp::Parameter& p) {
            m_info.led_count = static_cast<size_t>(p.as_int());
        },
        "Number of pixels in the LED strip");

    m_frame_sub = create_subscription<clover2_led::msg::LedFrame>(
        "led_frame", rclcpp::SystemDefaultsQoS(),
        [this](const clover2_led::msg::LedFrame& msg) { on_frame(msg); });
}

driver::~driver() = default;

void driver::on_frame(const clover2_led::msg::LedFrame& msg) {
    if (!m_device) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "LED driver: no device attached, dropping frame");
        return;
    }

    const data::led_frame frame{msg};
    m_device->write(frame);
}

}  // namespace clover2_led

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_led::driver)
