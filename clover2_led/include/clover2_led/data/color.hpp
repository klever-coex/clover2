#pragma once

#include <clover2_led/msg/color.hpp>

#include <cmath>

namespace clover2_led::data {

struct color {
    constexpr color() = default;
    constexpr color(uint8_t r, uint8_t g, uint8_t b, float brightness = 1.0)
        : r(r)
        , g(g)
        , b(b)
        , brightness(brightness) {}

    constexpr color(const clover2_led::msg::Color& color)
        : r(color.r)
        , g(color.g)
        , b(color.b)
        , brightness(color.brightness) {}

    [[nodiscard]]
    clover2_led::msg::Color to_msg() const {
        clover2_led::msg::Color msg;
        msg.r = r;
        msg.g = g;
        msg.b = b;
        msg.brightness = brightness;
        return msg;
    }

    bool operator==(const color& other) const {
        return other.r == r && other.g == g && other.b == b &&
               (std::abs(other.brightness - brightness) < 1e-5f);
    }

    uint8_t r{0}, g{0}, b{0};
    float brightness{1.0};
};

}  // namespace clover2_led::data
