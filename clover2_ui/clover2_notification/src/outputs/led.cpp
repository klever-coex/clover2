#include <clover2_led/client.hpp>
#include <clover2_notification/output.hpp>
#include <pluginlib/class_list_macros.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_notification::outputs {

/**
 * @class led
 * @brief Notification output that displays events using LED animations.
 *
 * The output selects animation configuration by event name override first and
 * by event priority second. Supported animations are handled by
 * clover2_led::client.
 */
class led final : public clover2_notification::output {
public:
    /** @brief Construct an LED notification output. */
    led() = default;

    /** @brief Destroy an LED notification output. */
    ~led() override = default;

    /**
     * @brief Cancel active timers, clear queued events, and turn LEDs off.
     */
    void clear() override;

private:
    /**
     * @brief Initialize LED-specific resources and configuration.
     *
     * Creates a LED client, declares output parameters, and loads default and
     * per-diagnostic animation configurations.
     *
     * @param node Lifecycle node that owns the output plugin.
     */
    void on_initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) override;

    /** @brief LED animation parameters used for one notification event. */
    struct animation_config {
        /** @brief Animation name supported by clover2_led::client. */
        std::string animation{"blink"};

        /** @brief Primary animation color. */
        clover2_led::data::color color{255, 255, 255};

        /** @brief Animation brightness in the range expected by LED driver. */
        float brightness{1.0F};

        /** @brief Animation period in seconds. */
        float period{1.0F};

        /** @brief Animation duration in seconds. */
        float duration{3.0F};
    };

    /**
     * @brief Declare and read one animation configuration block.
     *
     * @param node Lifecycle node used to declare parameters.
     * @param prefix Parameter prefix for the animation block.
     * @param defaults Default values used for missing parameters.
     * @return Loaded animation configuration.
     *
     * @throws std::invalid_argument if configured duration is not positive.
     * @throws std::runtime_error if configured color does not contain three
     * components.
     */
    animation_config declare_animation_config(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
        const std::string& prefix, const animation_config& defaults) const;

    /**
     * @brief Load default priority-based and name-based override animations.
     *
     * @param node Lifecycle node used to declare and read parameters.
     */
    void load_animation_configs(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node);

    /**
     * @brief Find animation configuration for an event.
     *
     * @param event Notification event to resolve.
     * @return Pointer to matching animation configuration, or nullptr if none
     * exists.
     */
    const animation_config* find_animation_config(
        const data::event& event) const;

    /**
     * @brief Start the configured LED animation.
     *
     * @param config Animation configuration to execute.
     *
     * @throws std::runtime_error if the animation name is unsupported or the
     * LED client call fails.
     */
    void start_animation(const animation_config& config) const;

    /**
     * @brief Process one notification event as an LED animation.
     *
     * @param event Notification event to display.
     * @param done Completion callback called when display duration expires or
     * when processing cannot be started.
     */
    void process_event(const data::event& event, done_callback done) override;

    std::string m_base_path{"led_strip"};
    rclcpp_lifecycle::LifecycleNode::SharedPtr m_node;
    std::shared_ptr<clover2_led::client> m_client;
    rclcpp::TimerBase::SharedPtr m_timer;
    rclcpp::Logger m_logger{rclcpp::get_logger("notification_led_output")};
    std::unordered_map<int, animation_config> m_default_animations;
    std::unordered_map<std::string, animation_config> m_override_animations;
};

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

}  // namespace

void led::on_initialize(
    const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) {
    m_logger = node->get_logger().get_child("led_output").get_child(id());
    m_node = node;

    m_base_path = node->declare_parameter<std::string>(id() + ".base_path", id());
    m_client = std::make_shared<clover2_led::client>(node, m_base_path);
    load_animation_configs(node);
}

void led::process_event(const data::event& event, done_callback done) {
    if (!m_client) {
        RCLCPP_WARN(m_logger, "LED client is not initialized");
        done();
        return;
    }

    const auto* config = find_animation_config(event);
    if (!config) {
        RCLCPP_WARN(m_logger,
                    "No LED animation configured for event '%s' "
                    "with priority %d",
                    event.name.c_str(), event.priority);
        done();
        return;
    }

    try {
        RCLCPP_INFO(m_logger,
                    "Start LED animation: reason='%s' name='%s' message='%s' "
                    "priority=%d queued=%zu output='%s' base_path='%s' "
                    "animation='%s' duration=%.2fs",
                    event.source.c_str(), event.name.c_str(),
                    event.message.c_str(), event.priority, queued_size(),
                    id().c_str(), m_base_path.c_str(), config->animation.c_str(),
                    config->duration);
        start_animation(*config);
    } catch (const std::exception& e) {
        RCLCPP_ERROR(m_logger, "Failed to start LED animation for '%s': %s",
                     event.name.c_str(), e.what());
        done();
        return;
    }

    const auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::duration<double>(config->duration));
    m_timer = m_node->create_wall_timer(duration, [this, done]() {
        if (m_timer) {
            m_timer->cancel();
            m_timer.reset();
        }
        done();
    });
}

void led::clear() {
    if (m_timer) {
        m_timer->cancel();
        m_timer.reset();
    }
    output::clear();
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

    if (config.duration <= 0.0F) {
        throw std::invalid_argument(
            "LED animation duration must be greater than zero: " + prefix);
    }

    return config;
}

void led::load_animation_configs(
    const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) {
    m_default_animations.clear();
    m_override_animations.clear();

    const animation_config warn_defaults{
        "blink", {255, 255, 0}, 0.7F, 1.0F, 3.0F};
    const animation_config error_defaults{
        "blink", {255, 0, 0}, 1.0F, 0.3F, 3.0F};
    const animation_config stale_defaults{
        "blink", {0, 0, 255}, 0.7F, 1.5F, 3.0F};

    const auto load_default = [&](const std::string& name,
                                  const animation_config& defaults,
                                  int default_priority) {
        const auto prefix = id() + ".defaults." + name;
        const auto priority = node->declare_parameter<int>(prefix + ".priority",
                                                           default_priority);
        m_default_animations.emplace(
            priority, declare_animation_config(node, prefix, defaults));
    };

    load_default("warn", warn_defaults, 1);
    load_default("error", error_defaults, 2);
    load_default("stale", stale_defaults, 3);

    const auto override_names =
        node->declare_parameter<std::vector<std::string>>(
            id() + ".override_names", std::vector<std::string>{});
    for (const auto& override_name : override_names) {
        const auto prefix = id() + ".overrides." + override_name;
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

    const auto default_it = m_default_animations.find(event.priority);
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
