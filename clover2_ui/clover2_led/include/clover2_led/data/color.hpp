#pragma once

#include <clover2_led_msgs/msg/color.hpp>

#include <cmath>

namespace clover2_led::data {

struct color {
    constexpr color() = default;

    constexpr color(uint8_t r, uint8_t g, uint8_t b)
        : r(r)
        , g(g)
        , b(b){}

    constexpr color(const clover2_led_msgs::msg::Color& color)
        : r(color.r)
        , g(color.g)
        , b(color.b) {}

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

    uint8_t r{0}, g{0}, b{0};
};

}  // namespace clover2_led::data
