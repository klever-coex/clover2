#pragma once

#include <clover2_led/animation/base_animation.hpp>
#include <clover2_led/data/led_frame.hpp>

namespace clover2_led::animation {

class rainbow : public base_animation {
public:
    static constexpr const char* name = "rainbow";

    explicit rainbow(const Request& req, int led_count,
                     rclcpp::Clock::SharedPtr clock);

protected:
    std::optional<data::led_frame> on_update(double elapsed_sec) override;

private:
    data::led_frame m_frame;
    float m_period;
};

}  // namespace clover2_led::animation
