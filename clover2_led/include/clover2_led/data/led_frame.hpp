#pragma once

#include <clover2_led/data/color.hpp>
#include <clover2_led/msg/led_frame.hpp>

#include <vector>

namespace clover2_led::data {

struct led_frame {
    constexpr led_frame() = default;

    led_frame(const clover2_led::msg::LedFrame& frame)
        : pixels(frame.colors.begin(), frame.colors.end())
        , brightness(frame.brightness) {}

    clover2_led::msg::LedFrame to_msg() const {
        clover2_led::msg::LedFrame msg;
        msg.colors.reserve(pixels.size());
        for (const auto& pixel : pixels) {
            msg.colors.push_back(pixel.to_msg());
        }
        msg.brightness = brightness;
        return msg;
    }

    size_t size() const noexcept {
        return pixels.size();
    }

    bool empty() const noexcept {
        return pixels.empty();
    }

    color& operator[](size_t index) noexcept { return pixels[index]; }

    const color& operator[](size_t index) const noexcept {
        return pixels[index];
    }

    static led_frame filled(const clover2_led::data::color& c, size_t count) {
        led_frame ret;
        ret.pixels.resize(count);

        std::fill(ret.pixels.begin(), ret.pixels.end(), c);

        return ret;
    }

    std::vector<clover2_led::data::color> pixels{};
    float brightness{1.0F};
};

}  // namespace clover2_led::data
