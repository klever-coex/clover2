#pragma once

#include <clover2_camera_utils/data/coverage_progress.hpp>

namespace clover2_camera_utils::data {

enum class sample_rejection_reason {
    none,
    invalid_observation,
    duplicate,
};

struct sample_selection_result {
    bool accepted{false};
    sample_rejection_reason rejection_reason{sample_rejection_reason::none};
    coverage_progress progress;
};

}  // namespace clover2_camera_utils::data
