#include <clover2_led/device/base_device.hpp>
#include <clover2_led/exceptions.hpp>

#include <algorithm>
#include <cstddef>

namespace clover2_led::device {

base_device::base_device()
    : m_logger(rclcpp::get_logger("base_device"))
    , m_clock(nullptr)
    , m_node_context(nullptr) {}

void base_device::initialize(
    const std::string& name, size_t led_count,
    std::shared_ptr<clover2_common::node_context> node_context) {
    m_node_context = std::move(node_context);

    m_name = name;
    m_logger = m_node_context->get_logger().get_child(m_name);
    m_clock = m_node_context->get_node_clock_interface()->get_clock();

    on_initialize(led_count);

    m_frame_buffer.resize(m_info.led_count);
    m_last_write = std::chrono::steady_clock::now();
}

void base_device::cleanup() noexcept {
    on_cleanup();
    m_node_context.reset();
}

const data::driver_info& base_device::info() const noexcept { return m_info; }
data::driver_info& base_device::info() noexcept { return m_info; }

void base_device::write(const data::led_frame& frame) {
    if (frame.pixels.size() != m_info.led_count) {
        throw frame_size_mismatch(m_info.led_count, frame.pixels.size());
    }

    auto now = std::chrono::steady_clock::now();
    const auto frame_period =
        std::chrono::duration<double>(1.f / m_info.max_fps);
    if (now - m_last_write < frame_period) {
        throw data::frequency_to_high();
    }

    std::copy(frame.pixels.begin(), frame.pixels.end(), m_frame_buffer.begin());

    for (auto& pixel : m_frame_buffer) {
        pixel.r = static_cast<uint8_t>(pixel.r * m_brightness);
        pixel.g = static_cast<uint8_t>(pixel.g * m_brightness);
        pixel.b = static_cast<uint8_t>(pixel.b * m_brightness);
    }

    write_raw_frame(m_frame_buffer);

    m_last_write = now;
}

void base_device::set_brightness(float brightness) {
    m_brightness = std::clamp(brightness, 0.0f, 1.0f);
    set_hardware_brightness(std::clamp(brightness, 0.0f, 1.0f));
}

float base_device::brightness() const { return m_brightness; }

bool base_device::set_hardware_brightness(float /* brightness */) {
    return true;
}

const std::string& base_device::get_name() const { return m_name; }

rclcpp::Logger base_device::get_logger() const { return m_logger; }
rclcpp::Clock::SharedPtr base_device::get_clock() const { return m_clock; }

}  // namespace clover2_led::device
