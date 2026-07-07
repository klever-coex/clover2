#pragma once

#include <cstddef>

namespace clover2_led::data {

struct driver_info {
    bool rgbw;
    bool hardware_brightness;
    size_t led_count;
    double max_fps;
};

}  // namespace clover2_led::data
