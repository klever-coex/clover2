#include <clover2_display/data/display_frame.hpp>
#include <clover2_display/driver.hpp>
#include <rclcpp_components/register_node_macro.hpp>

#include <exception>
#include <string>

namespace clover2_display {

driver::driver(const rclcpp::NodeOptions& options)
    : clover2_common::node("display_driver", options) {
    enable_diagnostic_updater();

    clover2_common::util::safe_declare_and_get(
        this, "device_plugin", std::string("ssd1306_i2c"), m_plugin_class);

    try {
        auto node_context =
            std::make_shared<clover2_common::node_context>(*this);
        m_device = m_plugin_loader.createSharedInstance(m_plugin_class);
        m_device->initialize(m_plugin_class, node_context);

        RCLCPP_INFO(get_logger(), "Loaded display device plugin: %s",
                    m_plugin_class.c_str());
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(),
                     "Failed to load display device plugin '%s': %s",
                     m_plugin_class.c_str(), e.what());
    }

    m_image_sub = create_subscription<sensor_msgs::msg::Image>(
        "~/image", rclcpp::SystemDefaultsQoS(),
        [this](const sensor_msgs::msg::Image& msg) { on_image(msg); });

    m_get_info_srv = create_service<clover2_display_msgs::srv::GetDriverInfo>(
        "~/get_driver_info",
        [this](
            const clover2_display_msgs::srv::GetDriverInfo::Request::SharedPtr
                req,
            clover2_display_msgs::srv::GetDriverInfo::Response::SharedPtr
                resp) { handle_get_driver_info(req, resp); });
}

driver::~driver() = default;

void driver::on_image(const sensor_msgs::msg::Image& msg) {
    if (!m_device) {
        RCLCPP_WARN_THROTTLE(
            get_logger(), *get_clock(), 5000,
            "Display driver: no device attached, dropping image");
        return;
    }

    try {
        m_device->write(data::display_frame{msg});
    } catch (const std::exception& e) {
        RCLCPP_WARN_THROTTLE(get_logger(), *get_clock(), 5000,
                             "Display driver: failed to write image: %s",
                             e.what());
    }
}

void driver::handle_get_driver_info(
    const clover2_display_msgs::srv::GetDriverInfo::Request::
        SharedPtr /* req */,
    clover2_display_msgs::srv::GetDriverInfo::Response::SharedPtr resp) const {
    if (!m_device) {
        resp->success = false;
        resp->message = "Display driver: no device attached";
        return;
    }

    const auto& dev = *m_device;
    const auto& info = dev.info();
    resp->success = true;
    resp->message = "ok";
    resp->width = info.width;
    resp->height = info.height;
    resp->max_fps = info.max_fps;
    resp->color_model = info.color_model;
    resp->supported_encodings = info.supported_encodings;
}

}  // namespace clover2_display

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_display::driver)
