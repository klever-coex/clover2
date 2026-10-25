#include <clover2_led/animation/base_animation.hpp>

namespace clover2_led::animation {

base_animation::base_animation(const Request& req, int led_count,
                               rclcpp::Clock::SharedPtr clock)
    : m_animation_name(req.animation_name)
    , m_led_count(led_count)
    , m_brightness(req.brightness)
    , m_duration(req.duration)
    , m_start_time(clock->now())
    , m_clock(std::move(clock)) {}

std::optional<data::led_frame> base_animation::update(double elapsed_sec) {
    auto frame = on_update(elapsed_sec);
    if (frame.has_value()) {
        frame->brightness = m_brightness;
    }
    return frame;
}

bool base_animation::is_finished() const {
    if (m_duration > 0.0F) {
        auto elapsed = (m_clock->now() - m_start_time).seconds();
        if (elapsed >= static_cast<double>(m_duration)) {
            return true;
        }
    }
    return is_animation_finished();
}

}  // namespace clover2_led::animation
