#include <clover2_led/animation/rainbow.hpp>
#include <clover2_led/data/color.hpp>

#include <cmath>

namespace clover2_led::animation {

rainbow::rainbow(const Request& req, int led_count,
                 rclcpp::Clock::SharedPtr clock)
    : base_animation(req, led_count, std::move(clock))
    , m_period(req.period) {
    m_frame.pixels.resize(led_count, data::color{0, 0, 0});
}

std::optional<data::led_frame> rainbow::on_update(double elapsed_sec) {
    double counter =
        std::fmod(elapsed_sec / static_cast<double>(m_period) * 360.0, 360.0);

    int count = led_count();
    for (int i = 0; i < count; i++) {
        double hue = std::fmod(counter + (360.0 * i / count), 360.0);
        m_frame.pixels[i] = data::color::from_hue(hue, 1.0, 1.0);
    }

    return m_frame;
}

}  // namespace clover2_led::animation
