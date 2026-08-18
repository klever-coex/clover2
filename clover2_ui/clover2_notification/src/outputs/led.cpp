#include <clover2_notification/outputs/led.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <pluginlib/class_list_macros.hpp>

#include <algorithm>
#include <stdexcept>
#include <vector>

namespace clover2_notification::outputs {

namespace {

uint8_t to_color_component(int64_t value) {
    return static_cast<uint8_t>(std::clamp<int64_t>(value, 0, 255));
}

std::vector<int64_t> to_color_array(const clover2_led::data::color& color) {
    return {color.r, color.g, color.b};
}

clover2_led::data::color parse_color(const std::vector<int64_t>& values) {
    if (values.size() != 3) {
        throw std::runtime_error("LED animation color should contain 3 values");
    }

    return {to_color_component(values[0]), to_color_component(values[1]),
            to_color_component(values[2])};
}

std::string level_name(uint8_t level) {
    switch (level) {
        case diagnostic_msgs::msg::DiagnosticStatus::WARN:
            return "warn";
        case diagnostic_msgs::msg::DiagnosticStatus::ERROR:
            return "error";
        case diagnostic_msgs::msg::DiagnosticStatus::STALE:
            return "stale";
        default:
            return "ok";
    }
}

}  // namespace

void led::initialize(const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) {
    if (!node) {
        throw std::invalid_argument("LED output received a null node");
    }

    m_logger = node->get_logger().get_child("led_output");

    const auto base_path =
        node->declare_parameter<std::string>("led.base_path", "led_strip");
    m_client = std::make_shared<clover2_led::client>(node, base_path);
    load_animation_configs(node);
}

void led::show(const data::event& event) {
    if (!m_client) {
        RCLCPP_WARN(m_logger, "LED client is not initialized");
        return;
    }

    const auto* config = find_animation_config(event);
    if (!config) {
        RCLCPP_WARN(m_logger,
                    "No LED animation configured for diagnostic '%s' level '%s'",
                    event.name.c_str(), level_name(event.level).c_str());
        return;
    }

    try {
        start_animation(*config);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(m_logger, "Failed to start LED animation for '%s': %s",
                     event.name.c_str(), e.what());
    }
}

void led::clear() {
    if (m_client) {
        m_client->clear();
    }
}

led::animation_config led::declare_animation_config(
    const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
    const std::string& prefix, const animation_config& defaults) const {
    animation_config config;
    config.animation = node->declare_parameter<std::string>(
        prefix + ".animation", defaults.animation);
    config.color = parse_color(node->declare_parameter<std::vector<int64_t>>(
        prefix + ".color", to_color_array(defaults.color)));
    config.brightness = static_cast<float>(node->declare_parameter<double>(
        prefix + ".brightness", defaults.brightness));
    config.period = static_cast<float>(
        node->declare_parameter<double>(prefix + ".period", defaults.period));
    config.duration = static_cast<float>(node->declare_parameter<double>(
        prefix + ".duration", defaults.duration));

    return config;
}

void led::load_animation_configs(
    const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) {
    m_default_animations.clear();
    m_override_animations.clear();

    const animation_config warn_defaults{"blink", {255, 255, 0}, 0.7F, 1.0F,
                                         0.0F};
    const animation_config error_defaults{"blink", {255, 0, 0}, 1.0F, 0.3F,
                                          0.0F};
    const animation_config stale_defaults{"blink", {0, 0, 255}, 0.7F, 1.5F,
                                          0.0F};

    m_default_animations.emplace(
        diagnostic_msgs::msg::DiagnosticStatus::WARN,
        declare_animation_config(node, "led.defaults.warn", warn_defaults));
    m_default_animations.emplace(
        diagnostic_msgs::msg::DiagnosticStatus::ERROR,
        declare_animation_config(node, "led.defaults.error", error_defaults));
    m_default_animations.emplace(
        diagnostic_msgs::msg::DiagnosticStatus::STALE,
        declare_animation_config(node, "led.defaults.stale", stale_defaults));

    const auto override_names = node->declare_parameter<std::vector<std::string>>(
        "led.override_names", std::vector<std::string>{});
    for (const auto& override_name : override_names) {
        const auto prefix = "led.overrides." + override_name;
        const auto diagnostic_name = node->declare_parameter<std::string>(
            prefix + ".diagnostic_name", override_name);
        auto config = declare_animation_config(node, prefix, warn_defaults);
        m_override_animations.emplace(diagnostic_name, config);

        RCLCPP_INFO(m_logger, "Loaded LED override '%s' for diagnostic '%s'",
                    override_name.c_str(), diagnostic_name.c_str());
    }
}

const led::animation_config* led::find_animation_config(
    const data::event& event) const {
    const auto override_it = m_override_animations.find(event.name);
    if (override_it != m_override_animations.end()) {
        return &override_it->second;
    }

    const auto default_it = m_default_animations.find(event.level);
    if (default_it != m_default_animations.end()) {
        return &default_it->second;
    }

    return nullptr;
}

void led::start_animation(const animation_config& config) const {
    if (config.animation == "solid_color") {
        m_client->solid_color(config.color, config.brightness, config.duration);
    } else if (config.animation == "blink") {
        m_client->blink(config.color, config.period, config.brightness,
                        config.duration);
    } else if (config.animation == "rainbow") {
        m_client->rainbow(config.period, config.brightness, config.duration);
    } else {
        throw std::runtime_error("Unsupported LED animation: " +
                                 config.animation);
    }
}

}  // namespace clover2_notification::outputs

PLUGINLIB_EXPORT_CLASS(clover2_notification::outputs::led,
                       clover2_notification::output)
