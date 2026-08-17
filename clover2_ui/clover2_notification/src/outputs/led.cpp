#include <clover2_notification/outputs/led.hpp>
#include <pluginlib/class_list_macros.hpp>

#include <stdexcept>

namespace clover2_notification::outputs {

void led::initialize(const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) {
    if (!node) {
        throw std::invalid_argument("LED output received a null node");
    }

    const auto base_path =
        node->declare_parameter<std::string>("led.base_path", "led_strip");
    m_client = std::make_shared<clover2_led::client>(node, base_path);
}

void led::show(const std::string& notification_name) {
    // TODO: define notification behavior.
    //  next for example: m_client->solid_color()
    (void)notification_name;
}

void led::clear() {
    if (m_client) {
        m_client->clear();
    }
}

}  // namespace clover2_notification::outputs

PLUGINLIB_EXPORT_CLASS(clover2_notification::outputs::led,
                       clover2_notification::output)
