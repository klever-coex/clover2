#include <clover2_led/animation/solid_color.hpp>

namespace clover2_led::animation {

solid_color::solid_color(const Request& req, int led_count,
                         rclcpp::Clock::SharedPtr clock)
    : base_animation(req, led_count, std::move(clock)) {
    if (!req.colors.empty()) {
        m_color = data::color(req.colors[0]);
    }
}

std::optional<data::led_frame> solid_color::on_update(double /*elapsed_sec*/) {
    if (m_frame_sent) {
        return std::nullopt;
    }

    m_frame_sent = true;
    return data::led_frame::filled(m_color, led_count());
}

}  // namespace clover2_led::animation
