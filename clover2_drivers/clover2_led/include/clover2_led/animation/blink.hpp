#pragma once

#include <clover2_led/animation/base_animation.hpp>

namespace clover2_led::animation {

class blink : public base_animation {
public:
    static constexpr const char* name = "blink";

    explicit blink(const Request& req, int led_count,
                   rclcpp::Clock::SharedPtr clock);

protected:
    std::optional<data::led_frame> on_update(double elapsed_sec) override;

private:
    data::color m_color;
    float m_period;
    int m_last_cycle{-1};
};

}  // namespace clover2_led::animation
