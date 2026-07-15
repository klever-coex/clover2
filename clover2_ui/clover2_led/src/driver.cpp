#include <clover2_led/driver.hpp>

// clover2
#include <clover2_led/animation_server.hpp>
#include <clover2_led/data/led_frame.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

#include <stdexcept>
#include <string>

namespace clover2_led {

driver::driver(const rclcpp::NodeOptions& options)
    : clover2_common::node("led_driver", options) {
    enable_diagnostic_updater();

    clover2_common::util::safe_declare_and_get(
        this, "device_plugin", std::string("ws2812_spi"), m_plugin_class);

    declare_parameter("led_count", 10);

    declare_and_watch_parameter<double>(
        "brightness_scale", 1.0,
        [this](const rclcpp::Parameter& p) {
            auto n = p.as_double();
            if (n < 0.05 || n > 1.0) {
                throw std::runtime_error(
                    "brightness_scale should be [0.05, 1.0]");
            }

            m_brightness_scale = n;
        },
        "Master brightness [0.05, 1.0]");

    try {
        if (m_device) {
            m_device->cleanup();
            m_device.reset();
        }

        auto node_context =
            std::make_shared<clover2_common::node_context>(*this);
        m_device = m_plugin_loader.createSharedInstance(m_plugin_class);
        m_device->initialize(m_plugin_class,
                             get_parameter("led_count").as_int(), node_context);

        RCLCPP_INFO(get_logger(), "Loaded LED device plugin: %s",
                    m_plugin_class.c_str());
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Failed to load LED device plugin '%s': %s",
                     m_plugin_class.c_str(), e.what());
    }

    if (m_device) {
        const auto& dev = *m_device;
        const auto& info = dev.info();
        m_animation_server = std::make_unique<animation_server>(
            static_cast<int>(info.led_count), m_device, get_logger(),
            get_clock());

        auto period = std::chrono::duration<double>(1.05 / info.max_fps);
        m_animation_timer =
            create_wall_timer(period, [this]() { animation_timer_callback(); });

        RCLCPP_INFO(get_logger(),
                    "Animation system ready: led_count=%zu, max_fps=%.1f, "
                    "timer_period=%.1fms",
                    info.led_count, info.max_fps, period.count() * 1000.0);
    }

    m_frame_sub = create_subscription<clover2_led_msgs::msg::LedFrame>(
        "~/led_frame", rclcpp::SystemDefaultsQoS(),
        [this](const clover2_led_msgs::msg::LedFrame& msg) { on_frame(msg); });

    m_get_info_srv = create_service<clover2_led_msgs::srv::GetDriverInfo>(
        "~/get_driver_info",
        [this](
            const clover2_led_msgs::srv::GetDriverInfo::Request::SharedPtr req,
            clover2_led_msgs::srv::GetDriverInfo::Response::SharedPtr resp) {
            handle_get_driver_info(req, resp);
        });

    m_get_frame_srv = create_service<clover2_led_msgs::srv::GetCurrentFrame>(
        "~/get_current_frame",
        [this](
            const clover2_led_msgs::srv::GetCurrentFrame::Request::SharedPtr
                req,
            clover2_led_msgs::srv::GetCurrentFrame::Response::SharedPtr resp) {
            handle_get_current_frame(req, resp);
        });

    m_start_animation_srv = create_service<
        clover2_led_msgs::srv::StartAnimation>(
        "~/start_animation",
        [this](
            const clover2_led_msgs::srv::StartAnimation::Request::SharedPtr req,
            clover2_led_msgs::srv::StartAnimation::Response::SharedPtr resp) {
            handle_start_animation(req, resp);
        });
}

driver::~driver() = default;

void driver::on_frame(const clover2_led_msgs::msg::LedFrame& msg) {
    if (!m_device) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "LED driver: no device attached, dropping frame");
        return;
    }

    if (m_animation_server) {
        m_animation_server->cancel();
    }

    m_last_frame = data::led_frame{msg};
    m_last_frame.brightness *= m_brightness_scale;
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

void driver::handle_start_animation(
    const clover2_led_msgs::srv::StartAnimation::Request::SharedPtr req,
    clover2_led_msgs::srv::StartAnimation::Response::SharedPtr resp) {
    if (!m_animation_server) {
        resp->success = false;
        resp->message = "Animation system not available (no device attached)";
        return;
    }

    try {
        auto r = req;
        r->brightness *= m_brightness_scale;
        m_animation_server->start(*r);
        resp->success = true;
        resp->message = "ok";
    } catch (const std::runtime_error& e) {
        resp->success = false;
        resp->message = e.what();
    }
}

void driver::animation_timer_callback() {
    if (m_animation_server) {
        m_animation_server->tick();
    }
}

}  // namespace clover2_led

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_led::driver)
