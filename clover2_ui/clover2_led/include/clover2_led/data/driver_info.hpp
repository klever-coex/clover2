#pragma once

#include <cstddef>

namespace clover2_led::data {

struct driver_info {
    size_t led_count{0};
    double max_fps{1.0};
};

}  // namespace clover2_led::data
