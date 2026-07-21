#include <clover2_led/animation/blink.hpp>

namespace clover2_led::animation {

blink::blink(const Request& req, int led_count, rclcpp::Clock::SharedPtr clock)
    : base_animation(req, led_count, std::move(clock))
    , m_period(req.period) {
    if (!req.colors.empty()) {
        m_color = data::color(req.colors[0]);
    }
}

std::optional<data::led_frame> blink::on_update(double elapsed_sec) {
    double half_period = m_period / 2.0;
    int cycle = static_cast<int>(elapsed_sec / half_period);

    if (cycle == m_last_cycle) {
        return std::nullopt;  // state unchanged
    }

    m_last_cycle = cycle;
    bool on = (cycle % 2) == 0;
    data::color c = on ? m_color : data::color{0, 0, 0};

    return data::led_frame::filled(c, led_count());
}

}  // namespace clover2_led::animation
