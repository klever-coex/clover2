#include <clover2_led/driver.hpp>

// clover2
#include <clover2_led/data/led_frame.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_led {

driver::driver(const rclcpp::NodeOptions& options)
    : clover2_common::node{"clover2_led_driver", options} {
    // deferred init — device subclasses aren't registered yet at construction
    m_init_timer = create_wall_timer(std::chrono::milliseconds(100), [this]() {
        m_init_timer->cancel();
        on_init();
    });
}

driver::~driver() = default;

void driver::on_init() {
    enable_parameter_watcher();
    enable_diagnostic_updater();

    declare_and_watch_parameter<double>(
        "brightness", 1.0,
        [this](const rclcpp::Parameter& p) {
            if (m_device) {
                m_device->set_brightness(static_cast<float>(p.as_double()));
            }
        },
        "Master brightness [0.0, 1.0]");

    // device parameters from param watcher
    declare_and_watch_parameter<int64_t>(
        "led_count", 0,
        [this](const rclcpp::Parameter& p) {
            m_info.led_count = static_cast<size_t>(p.as_int());
        },
        "Number of pixels in the LED strip");

    declare_and_watch_parameter<double>(
        "max_fps", 30.0,
        [this](const rclcpp::Parameter& p) { m_info.max_fps = p.as_double(); },
        "Maximum refresh rate in Hz");

    declare_and_watch_parameter<bool>(
        "rgbw", false,
        [this](const rclcpp::Parameter& p) { m_info.rgbw = p.as_bool(); },
        "RGBW mode");

    declare_and_watch_parameter<bool>(
        "hardware_brightness", false,
        [this](const rclcpp::Parameter& p) {
            m_info.hardware_brightness = p.as_bool();
        },
        "Hardware supports global brightness control");

    // TODO: create concrete device via pluginlib or factory
    // m_device = std::make_unique<...>(m_info);
    // m_device->initialize();

    m_frame_sub = create_subscription<clover2_led::msg::LedFrame>(
        "led_frame", rclcpp::SystemDefaultsQoS(),
        [this](const clover2_led::msg::LedFrame& msg) { on_frame(msg); });
}

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
