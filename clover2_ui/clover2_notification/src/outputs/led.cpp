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

namespace {

uint8_t to_color_component(int64_t value) {
    return static_cast<uint8_t>(std::clamp<int64_t>(value, 0, 255));
}

clover2_led::data::color parse_color(const std::vector<int64_t>& values) {
    if (values.size() != 3) {
        throw std::runtime_error("LED animation color should contain 3 values");
    }

    return {to_color_component(values[0]), to_color_component(values[1]),
            to_color_component(values[2])};
}

template <typename T>
T declare_required_parameter(
    const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
    const std::string& name) {
    try {
        return node->declare_parameter<T>(name);
    } catch (const std::exception& e) {
        throw std::runtime_error("Required LED parameter is not set: " + name);
    }
}

}  // namespace

/**
 * @class led
 * @brief Notification output that displays events using LED animations.
 *
 * The output selects animation configuration by event name override first and
 * by configured event priority reaction second. Supported animations are
 * handled by clover2_led::client.
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
    void clear() override {
        if (m_timer) {
            m_timer->cancel();
            m_timer.reset();
        }
        output::clear();
        if (m_client) {
            m_client->clear();
        }
    }

private:
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
     * @brief Declare and read one required animation configuration block.
     *
     * @param node Lifecycle node used to declare parameters.
     * @param prefix Parameter prefix for the animation block.
     * @return Loaded animation configuration.
     *
     * @throws std::invalid_argument if configured duration is not positive.
     * @throws std::runtime_error if a required parameter is missing or if
     * configured color does not contain three components.
     */
    animation_config declare_animation_config(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
        const std::string& prefix) const {
        animation_config config;
        config.animation = declare_required_parameter<std::string>(
            node, prefix + ".animation");
        config.color =
            parse_color(declare_required_parameter<std::vector<int64_t>>(
                node, prefix + ".color"));
        config.brightness = static_cast<float>(
            declare_required_parameter<double>(node, prefix + ".brightness"));
        config.period = static_cast<float>(
            declare_required_parameter<double>(node, prefix + ".period"));
        config.duration = static_cast<float>(
            declare_required_parameter<double>(node, prefix + ".duration"));

        if (config.duration <= 0.0F) {
            throw std::invalid_argument(
                "LED animation duration must be greater than zero: " + prefix);
        }

        return config;
    }

    /**
     * @brief Load priority-based reactions and name-based override animations.
     *
     * @param node Lifecycle node used to declare and read parameters.
     */
    void load_animation_configs(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) {
        m_reactions.clear();
        m_override_animations.clear();

        const auto reaction_names =
            node->declare_parameter<std::vector<std::string>>(
                id() + ".reaction_names", std::vector<std::string>{});
        for (const auto& reaction_name : reaction_names) {
            const auto prefix = id() + ".reactions." + reaction_name;
            const auto priority =
                declare_required_parameter<int>(node, prefix + ".priority");

            m_reactions.emplace(priority,
                                declare_animation_config(node, prefix));

            RCLCPP_INFO(m_logger, "Loaded LED reaction '%s' for priority %d",
                        reaction_name.c_str(), static_cast<int>(priority));
        }

        const auto override_names =
            node->declare_parameter<std::vector<std::string>>(
                id() + ".override_names", std::vector<std::string>{});
        for (const auto& override_name : override_names) {
            const auto prefix = id() + ".overrides." + override_name;
            const auto diagnostic_name =
                declare_required_parameter<std::string>(
                    node, prefix + ".diagnostic_name");
            auto config = declare_animation_config(node, prefix);
            m_override_animations.emplace(diagnostic_name, config);

            RCLCPP_INFO(m_logger,
                        "Loaded LED override '%s' for diagnostic '%s'",
                        override_name.c_str(), diagnostic_name.c_str());
        }
    }

    /**
     * @brief Find animation configuration for an event.
     *
     * @param event Notification event to resolve.
     * @return Pointer to matching animation configuration, or nullptr if none
     * exists.
     */
    const animation_config* find_animation_config(
        const data::event& event) const {
        const auto override_it = m_override_animations.find(event.name);
        if (override_it != m_override_animations.end()) {
            return &override_it->second;
        }

        const auto reaction_it = m_reactions.find(event.priority);
        if (reaction_it != m_reactions.end()) {
            return &reaction_it->second;
        }

        return nullptr;
    }

    /**
     * @brief Start the configured LED animation.
     *
     * @param config Animation configuration to execute.
     *
     * @throws std::runtime_error if the animation name is unsupported or the
     * LED client call fails.
     */
    void start_animation(const animation_config& config) const {
        if (config.animation == "solid_color") {
            m_client->solid_color(config.color, config.brightness,
                                  config.duration);
        } else if (config.animation == "blink") {
            m_client->blink(config.color, config.period, config.brightness,
                            config.duration);
        } else if (config.animation == "rainbow") {
            m_client->rainbow(config.period, config.brightness,
                              config.duration);
        } else {
            throw std::runtime_error("Unsupported LED animation: " +
                                     config.animation);
        }
    }

    /**
     * @brief Initialize LED-specific resources and configuration.
     *
     * Creates a LED client, declares output parameters, and loads reaction and
     * per-diagnostic animation configurations.
     *
     * @param node Lifecycle node that owns the output plugin.
     */
    void on_initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) override {
        m_logger = node->get_logger().get_child("led_output").get_child(id());
        m_node = node;

        m_base_path =
            node->declare_parameter<std::string>(id() + ".base_path", id());
        m_client = std::make_shared<clover2_led::client>(node, m_base_path);
        load_animation_configs(node);
    }

    /**
     * @brief Process one notification event as an LED animation.
     *
     * @param event Notification event to display.
     * @param done Completion callback called when display duration expires or
     * when processing cannot be started.
     */
    void process_event(const data::event& event, done_callback done) override {
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
            RCLCPP_INFO(
                m_logger,
                "Start LED animation: reason='%s' name='%s' message='%s' "
                "priority=%d queued=%zu output='%s' base_path='%s' "
                "animation='%s' duration=%.2fs",
                event.source.c_str(), event.name.c_str(), event.message.c_str(),
                event.priority, queued_size(), id().c_str(),
                m_base_path.c_str(), config->animation.c_str(),
                config->duration);
            start_animation(*config);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(m_logger, "Failed to start LED animation for '%s': %s",
                         event.name.c_str(), e.what());
            done();
            return;
        }

        const auto duration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double>(config->duration));
        m_timer = m_node->create_wall_timer(duration, [this, done]() {
            if (m_timer) {
                m_timer->cancel();
                m_timer.reset();
            }
            done();
        });
    }

    std::string m_base_path{"led_strip"};
    rclcpp_lifecycle::LifecycleNode::SharedPtr m_node;
    std::shared_ptr<clover2_led::client> m_client;
    rclcpp::TimerBase::SharedPtr m_timer;
    rclcpp::Logger m_logger{rclcpp::get_logger("notification_led_output")};
    std::unordered_map<int, animation_config> m_reactions;
    std::unordered_map<std::string, animation_config> m_override_animations;
};

}  // namespace clover2_notification::outputs

PLUGINLIB_EXPORT_CLASS(clover2_notification::outputs::led,
                       clover2_notification::output)
