#include <clover2_led/driver.hpp>

// clover2
#include <clover2_led/data/led_frame.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

#include <string>

namespace clover2_led {

driver::driver(const rclcpp::NodeOptions& options)
    : clover2_common::node("clover2_led_driver", options) {
    enable_diagnostic_updater();

    clover2_common::util::safe_declare_and_get(
        this, "device_plugin", std::string("clover2_led::device::ws2812_spi"),
        m_plugin_class);

    declare_parameter("led_count", 10);

    declare_and_watch_parameter<double>(
        "brightness", 0.5,
        [this](const rclcpp::Parameter& p) {
            if (m_device) {
                m_device->set_brightness(static_cast<float>(p.as_double()));
            }
        },
        "Master brightness [0.0, 1.0]");

    try {
        if (m_device) {
            m_device->cleanup();
            m_device.reset();
        }

        auto node_context =
            std::make_shared<clover2_common::node_context>(*this);
        m_device = m_plugin_loader.createUniqueInstance(m_plugin_class);
        m_device->initialize(m_plugin_class,
                             get_parameter("led_count").as_int(), node_context);

        RCLCPP_INFO(get_logger(), "Loaded LED device plugin: %s",
                    m_plugin_class.c_str());
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Failed to load LED device plugin '%s': %s",
                     m_plugin_class.c_str(), e.what());
    }

    m_frame_sub = create_subscription<clover2_led_msgs::msg::LedFrame>(
        "led_frame", rclcpp::SystemDefaultsQoS(),
        [this](const clover2_led_msgs::msg::LedFrame& msg) { on_frame(msg); });

    m_get_info_srv = create_service<clover2_led_msgs::srv::GetDriverInfo>(
        "get_driver_info",
        [this](
            const clover2_led_msgs::srv::GetDriverInfo::Request::SharedPtr req,
            clover2_led_msgs::srv::GetDriverInfo::Response::SharedPtr resp) {
            handle_get_driver_info(req, resp);
        });

    m_get_frame_srv = create_service<clover2_led_msgs::srv::GetCurrentFrame>(
        "get_current_frame",
        [this](
            const clover2_led_msgs::srv::GetCurrentFrame::Request::SharedPtr
                req,
            clover2_led_msgs::srv::GetCurrentFrame::Response::SharedPtr resp) {
            handle_get_current_frame(req, resp);
        });
}

driver::~driver() = default;

void driver::on_frame(const clover2_led_msgs::msg::LedFrame& msg) {
    if (!m_device) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "LED driver: no device attached, dropping frame");
        return;
    }

    m_last_frame = data::led_frame{msg};
    m_device->write(m_last_frame);
}

void driver::handle_get_driver_info(
    const clover2_led_msgs::srv::GetDriverInfo::Request::SharedPtr /* req */,
    clover2_led_msgs::srv::GetDriverInfo::Response::SharedPtr resp) const {
    if (!m_device) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "LED driver: no device attached, dropping frame");
        resp->success = false;
        resp->message = "LED driver: no device attached, dropping frame";
        return;
    }

    const auto& dev = *m_device;
    const clover2_led::data::driver_info& info = dev.info();
    resp->success = true;
    resp->message = "ok";
    resp->led_count = static_cast<uint32_t>(info.led_count);
    resp->max_fps = info.max_fps;
}

void driver::handle_get_current_frame(
    const clover2_led_msgs::srv::GetCurrentFrame::Request::SharedPtr /* req */,
    clover2_led_msgs::srv::GetCurrentFrame::Response::SharedPtr resp) {
    if (!m_device) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "LED driver: no device attached, dropping frame");
        resp->success = false;
        resp->message = "LED driver: no device attached, dropping frame";
        return;
    }

    resp->success = true;
    resp->message = "ok";
    resp->brightness = m_last_frame.brightness;
    resp->colors.reserve(m_last_frame.pixels.size());

    for (const auto& pixel : m_last_frame.pixels) {
        resp->colors.push_back(pixel.to_msg());
    }
}

}  // namespace clover2_led

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_led::driver)
