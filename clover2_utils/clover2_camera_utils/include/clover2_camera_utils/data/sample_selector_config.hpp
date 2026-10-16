#pragma once

#include <array>
#include <cstddef>

namespace clover2_camera_utils::data {

struct sample_selector_config {
    std::array<double, 4> required_ranges{0.7, 0.7, 0.4, 0.5};
    double duplicate_distance_threshold{0.2};
    size_t minimum_samples_for_completion{20};
};

}  // namespace clover2_camera_utils::data
