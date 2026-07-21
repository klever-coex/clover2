#include <clover2_led/animation_server.hpp>

// clover2
#include <clover2_led/animation/factory.hpp>
#include <clover2_led/data/color.hpp>
#include <rclcpp/logging.hpp>

namespace clover2_led {

animation_server::animation_server(int led_count,
                                   std::shared_ptr<device::base_device> device,
                                   rclcpp::Logger logger,
                                   rclcpp::Clock::SharedPtr clock)
    : m_led_count(led_count)
    , m_device(std::move(device))
    , m_logger(std::move(logger))
    , m_clock(std::move(clock)) {}

void animation_server::tick() {
    if (!m_animation) {
        return;
    }

    if (m_animation->is_finished()) {
        RCLCPP_DEBUG(m_logger, "Animation finished, clearing strip");
        m_animation.reset();
        try {
            m_device->write(
                data::led_frame::filled(data::color{0, 0, 0}, m_led_count));
        } catch (const std::exception& e) {
            RCLCPP_ERROR(m_logger, "Failed to clear strip: %s", e.what());
        }
        return;
    }

    double elapsed_sec = (m_clock->now() - m_animation->start_time()).seconds();
    auto frame = m_animation->update(elapsed_sec);
    if (frame.has_value()) {
        m_last_frame = *frame;
        try {
            m_device->write(*frame);
        } catch (const std::exception& e) {
            RCLCPP_ERROR(m_logger, "Animation write failed: %s", e.what());
        }
    }
}

void animation_server::start(const animation::base_animation::Request& req) {
    m_animation = animation::factory::instance().create(req.animation_name, req,
                                                       m_led_count, m_clock);

    RCLCPP_DEBUG(m_logger, "Started animation: %s", req.animation_name.c_str());
}

void animation_server::cancel() noexcept {
    if (!m_animation) {
        return;
    }

    RCLCPP_DEBUG(m_logger, "Animation cancelled");
    m_animation.reset();

    try {
        m_device->write(
            data::led_frame::filled(data::color{0, 0, 0}, m_led_count));
    } catch (...) {
    }
}

bool animation_server::active() const { return m_animation != nullptr; }

}  // namespace clover2_led
