/**
 * @file coverage_progress.hpp
 * @brief Provides calibration dataset coverage progress.
 */

#pragma once

#include <clover2_camera_utils/data/feature_values.hpp>

namespace clover2_camera_utils::data {

/** @brief Describes calibration dataset coverage by sample features. */
struct coverage_progress {
    feature_values features;  ///< Coverage by each sample feature.
    bool data_sufficient{
        false};  ///< Whether data is sufficient for calibration.
};

}  // namespace clover2_camera_utils::data
