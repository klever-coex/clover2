/**
 * @file led.hpp
 * @brief Provides LED-strip notification output plugin.
 */

#pragma once

// clover2
#include <clover2_led/client.hpp>
#include <clover2_notification/output.hpp>

// STL
#include <memory>
#include <string>
#include <unordered_map>

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
     * @brief Initialize the LED output instance.
     *
     * Creates a LED client, declares output parameters, and loads default and
     * per-diagnostic animation configurations.
     *
     * @param node Lifecycle node that owns the output plugin.
     * @param id Output instance identifier used as a parameter namespace.
     *
     * @throws std::invalid_argument if @p node is null or @p id is empty.
     */
    void initialize(const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
                    const std::string& id) override;

    /**
     * @brief Cancel active timers, clear queued events, and turn LEDs off.
     */
    void clear() override;

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

    std::string m_id{"led_strip"};
    std::string m_base_path{"led_strip"};
    rclcpp_lifecycle::LifecycleNode::SharedPtr m_node;
    rclcpp::CallbackGroup::SharedPtr m_client_callback_group;
    std::shared_ptr<clover2_led::client> m_client;
    rclcpp::TimerBase::SharedPtr m_timer;
    rclcpp::Logger m_logger{rclcpp::get_logger("notification_led_output")};
    std::unordered_map<int, animation_config> m_default_animations;
    std::unordered_map<std::string, animation_config> m_override_animations;
};

}  // namespace clover2_notification::outputs
