#include <clover2_common/node_context.hpp>
#include <clover2_led/client.hpp>
#include <clover2_notification/output.hpp>
#include <pluginlib/class_list_macros.hpp>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace clover2_notification::outputs {

namespace {

template <typename T>
T declare_required_parameter(
    const std::shared_ptr<clover2_common::node_context>& node_context,
    const std::string& name) {
    try {
        auto value =
            rclcpp::node_interfaces::get_node_parameters_interface(node_context)
                ->declare_parameter(name, rclcpp::ParameterValue(T{}))
                .get<T>();
        if constexpr (std::is_same_v<T, std::string>) {
            if (value.empty()) {
                throw std::runtime_error("empty string");
            }
        } else if (value == T{}) {
            throw std::runtime_error("default value");
        }
        return value;
    } catch (const std::exception&) {
        throw std::runtime_error("Required LED parameter is not set: " + name);
    }
}

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

}  // namespace

/**
 * @class led
 * @brief Notification output that displays events using LED animations.
 *
 * The output selects animation configuration by exact event name. Supported
 * animations are handled by clover2_led::client.
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
    /** @brief LED reaction parameters used for one notification event. */
    struct reaction_config {
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
     * @param node_context Shared node context used to declare parameters.
     * @param prefix Parameter prefix for the animation block.
     * @return Loaded reaction configuration.
     *
     * @throws std::invalid_argument if configured duration is not positive.
     * @throws std::runtime_error if a required parameter is missing or if
     * configured color does not contain three components.
     */
    reaction_config declare_reaction_config(
        const std::shared_ptr<clover2_common::node_context>& node_context,
        const std::string& prefix) const {
        reaction_config config;

        config.animation = declare_required_parameter<std::string>(
            node_context, prefix + ".animation");
        config.color =
            parse_color(declare_required_parameter<std::vector<int64_t>>(
                node_context, prefix + ".color"));
        config.brightness =
            static_cast<float>(declare_required_parameter<double>(
                node_context, prefix + ".brightness"));
        config.period = static_cast<float>(declare_required_parameter<double>(
            node_context, prefix + ".period"));
        config.duration = static_cast<float>(declare_required_parameter<double>(
            node_context, prefix + ".duration"));

        if (config.duration <= 0.0F) {
            throw std::invalid_argument(
                "LED animation duration must be greater than zero: " + prefix);
        }

        return config;
    }

    /**
     * @brief Load event-name-based reactions.
     *
     * @param node_context Shared node context used to declare and read
     * parameters.
     */
    void load_reaction_configs(
        const std::shared_ptr<clover2_common::node_context>& node_context) {
        m_reactions.clear();

        const auto reaction_names =
            declare_output_parameter<std::vector<std::string>>(
                "reaction_names", std::vector<std::string>{});
        for (const auto& reaction_name : reaction_names) {
            const auto prefix = id() + ".reactions." + reaction_name;
            const auto event_name = declare_required_parameter<std::string>(
                node_context, prefix + ".event_name");

            m_reactions.emplace(event_name,
                                declare_reaction_config(node_context, prefix));

            RCLCPP_INFO(m_logger, "Loaded LED reaction '%s' for event '%s'",
                        reaction_name.c_str(), event_name.c_str());
        }
    }

    /**
     * @brief Initialize LED-specific resources and configuration.
     *
     * Declares output parameters, loads event-name-based reaction
     * configurations, and creates a LED client.
     *
     */
    void on_initialize() override {
        m_logger = node_context()
                       ->get_logger()
                       .get_child("led_output")
                       .get_child(id());
        m_base_path = declare_output_parameter<std::string>("base_path", id());
        m_client =
            std::make_shared<clover2_led::client>(node_context(), m_base_path);
        load_reaction_configs(node_context());
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

        const auto config_it = m_reactions.find(event.name);
        if (config_it == m_reactions.end()) {
            RCLCPP_DEBUG(m_logger,
                         "No LED animation configured for event '%s' "
                         "with priority %d",
                         event.name.c_str(), event.priority);
            done();
            return;
        }

        const auto& config = config_it->second;

        try {
            RCLCPP_INFO(
                m_logger,
                "Start LED animation: reason='%s' name='%s' message='%s' "
                "priority=%d queued=%zu output='%s' base_path='%s' "
                "animation='%s' duration=%.2fs",
                event.source.c_str(), event.name.c_str(), event.message.c_str(),
                event.priority, queued_size(), id().c_str(),
                m_base_path.c_str(), config.animation.c_str(), config.duration);
            auto request = std::make_shared<
                clover2_led_msgs::srv::StartAnimation::Request>();
            request->animation_name = config.animation;
            request->brightness = config.brightness;
            request->period = config.period;
            request->duration = config.duration;
            request->colors.push_back(config.color.to_msg());
            m_client->call_animation(request);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(m_logger, "Failed to start LED animation for '%s': %s",
                         event.name.c_str(), e.what());
            done();
            return;
        }

        const auto duration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double>(config.duration));
        m_timer = create_timer(duration, [this, done]() {
            if (m_timer) {
                m_timer->cancel();
                m_timer.reset();
            }
            done();
        });
    }

    std::string m_base_path{"led_strip"};
    std::shared_ptr<clover2_led::client> m_client;
    rclcpp::TimerBase::SharedPtr m_timer;
    rclcpp::Logger m_logger{rclcpp::get_logger("notification_led_output")};
    std::unordered_map<std::string, reaction_config> m_reactions;
};

}  // namespace clover2_notification::outputs

PLUGINLIB_EXPORT_CLASS(clover2_notification::outputs::led,
                       clover2_notification::output)
