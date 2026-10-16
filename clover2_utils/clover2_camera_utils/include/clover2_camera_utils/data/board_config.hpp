#pragma once

#include <cstdint>

namespace clover2_camera_utils::data {

struct board_config {
    uint32_t rows{0};
    uint32_t columns{0};
    double square_size_m{0.0};
};

}  // namespace clover2_camera_utils::data
