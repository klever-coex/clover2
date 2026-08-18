#pragma once

#include <clover2_led/client.hpp>
#include <clover2_notification/output.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace clover2_notification::outputs {

class led final : public clover2_notification::output {
public:
    led() = default;
    ~led() override = default;

    void initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) override;
    void show(const data::event& event) override;
    void clear() override;

private:
    struct animation_config {
        std::string animation{"blink"};
        clover2_led::data::color color{255, 255, 255};
        float brightness{1.0F};
        float period{1.0F};
        float duration{0.0F};
    };

    animation_config declare_animation_config(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
        const std::string& prefix, const animation_config& defaults) const;
    void load_animation_configs(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node);
    const animation_config* find_animation_config(const data::event& event) const;
    void start_animation(const animation_config& config) const;

    std::shared_ptr<clover2_led::client> m_client;
    rclcpp::Logger m_logger{rclcpp::get_logger("notification_led_output")};
    std::unordered_map<uint8_t, animation_config> m_default_animations;
    std::unordered_map<std::string, animation_config> m_override_animations;
};

}  // namespace clover2_notification::outputs
