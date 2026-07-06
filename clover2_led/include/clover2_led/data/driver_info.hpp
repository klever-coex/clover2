#pragma once

namespace clover2_led::data {

struct driver_info {
    bool rgbw;
    bool hardware_brightness;
    size_t pixel_count;
    float max_fps;
};

}  // namespace clover2_led::data
