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

    void initialize(const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
                    const std::string& id) override;
    void clear() override;

private:
    struct animation_config {
        std::string animation{"blink"};
        clover2_led::data::color color{255, 255, 255};
        float brightness{1.0F};
        float period{1.0F};
        float duration{3.0F};
    };

    animation_config declare_animation_config(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
        const std::string& prefix, const animation_config& defaults) const;
    void load_animation_configs(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node);
    const animation_config* find_animation_config(
        const data::event& event) const;
    void start_animation(const animation_config& config) const;
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
