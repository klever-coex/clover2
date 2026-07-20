#pragma once

#include <clover2_led/animation/base_animation.hpp>

namespace clover2_led::animation {

class solid_color : public base_animation {
public:
    static constexpr const char* name = "solid_color";

    explicit solid_color(const Request& req, int led_count,
                         rclcpp::Clock::SharedPtr clock);

protected:
    std::optional<data::led_frame> on_update(double elapsed_sec) override;

private:
    data::color m_color;
    bool m_frame_sent{false};
};

}  // namespace clover2_led::animation
