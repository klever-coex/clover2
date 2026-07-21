#pragma once

// clover2
#include <clover2_led/data/color.hpp>
#include <clover2_led/data/led_frame.hpp>

// ROS2
#include <clover2_led_msgs/srv/start_animation.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>
#include <optional>
#include <string>

namespace clover2_led::animation {

class base_animation {
public:
    using Request = clover2_led_msgs::srv::StartAnimation::Request;
    using UniquePtr = std::unique_ptr<base_animation>;

    static constexpr const char* name = "base";

    explicit base_animation(const Request& req, int led_count,
                            rclcpp::Clock::SharedPtr clock);

    virtual ~base_animation() = default;

    // nullopt if no update needed
    std::optional<data::led_frame> update(double elapsed_sec);

    bool is_finished() const;

    const std::string& animation_name() const { return m_animation_name; }
    int led_count() const { return m_led_count; }
    float brightness() const { return m_brightness; }
    float duration() const { return m_duration; }
    rclcpp::Time start_time() const { return m_start_time; }
    rclcpp::Clock::SharedPtr clock() const { return m_clock; }

protected:
    virtual std::optional<data::led_frame> on_update(double elapsed_sec) = 0;
    virtual bool is_animation_finished() const { return false; }

private:
    std::string m_animation_name;
    int m_led_count;
    float m_brightness;
    float m_duration;
    rclcpp::Time m_start_time;
    rclcpp::Clock::SharedPtr m_clock;
};

}  // namespace clover2_led::animation
