/**
 * @file target_detection.hpp
 * @brief Provides the result of calibration target detection.
 */

#pragma once

#include <clover2_camera_utils/data/observation.hpp>
#include <clover2_camera_utils/data/sample_features.hpp>

namespace clover2_camera_utils::data {

/**
 * @brief Combines calibration correspondences with target-specific selection
 * features.
 */
struct target_detection {
    observation calibration;       ///< Corresponding image and object points.
    sample_features selection;     ///< Descriptor used by the sample selector.
};

}  // namespace clover2_camera_utils::data