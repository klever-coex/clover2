#pragma once

#include <clover2_led_msgs/msg/color.hpp>

#include <cmath>

namespace clover2_led::data {

struct color {
    constexpr color() = default;

    constexpr color(uint8_t r, uint8_t g, uint8_t b)
        : r(r)
        , g(g)
        , b(b) {}

    constexpr color(const clover2_led_msgs::msg::Color& color)
        : r(color.r)
        , g(color.g)
        , b(color.b) {}

    constexpr color(const color&) noexcept = default;
    constexpr color(color&&) noexcept = default;
    constexpr color& operator=(const color&) noexcept = default;
    constexpr color& operator=(color&&) noexcept = default;

    clover2_led_msgs::msg::Color to_msg() const {
        clover2_led_msgs::msg::Color msg;
        msg.r = r;
        msg.g = g;
        msg.b = b;
        return msg;
    }

    bool operator==(const color& other) const {
        return other.r == r && other.g == g && other.b == b;
    }

    static color from_hue(double hue, double saturation = 1.0,
                          double value = 0.0) {
        if (hue < 0.0f) hue = 360.0f + std::fmod(hue, 360.0f);
        if (hue >= 360.0f) hue = std::fmod(hue, 360.0f);

        int sector = static_cast<int>(std::floor(hue / 60.0)) % 6;
        double fractionalSector = (hue / 60.0) - std::floor(hue / 60.0);

        double p = value * (1.0f - saturation);
        double q = value * (1.0f - fractionalSector * saturation);
        double t = value * (1.0f - (1.0f - fractionalSector) * saturation);

        double r, g, b;

        // clang-format off
        switch (sector) {
            case 0: r = value; g = t;     b = p;     break;
            case 1: r = q;     g = value; b = p;     break;
            case 2: r = p;     g = value; b = t;     break;
            case 3: r = p;     g = q;     b = value; break;
            case 4: r = t;     g = p;     b = value; break;
            case 5: r = value; g = p;     b = q;     break;
            default: break;
        }
        // clang-format on

        color c{
            static_cast<uint8_t>(std::round(r * 255.0)),
            static_cast<uint8_t>(std::round(g * 255.0)),
            static_cast<uint8_t>(std::round(b * 255.0)),
        };

        return c;
    }

    uint8_t r{0}, g{0}, b{0};
};

}  // namespace clover2_led::data
