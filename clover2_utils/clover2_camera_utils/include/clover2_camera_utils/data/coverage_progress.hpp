/**
 * @file coverage_progress.hpp
 * @brief Provides calibration dataset coverage progress.
 */

#pragma once

namespace clover2_camera_utils::data {

/** @brief Describes calibration dataset coverage by sample features. */
struct coverage_progress {
    double x{0.0};               ///< Coverage by horizontal sample position.
    double y{0.0};               ///< Coverage by vertical sample position.
    double size{0.0};            ///< Coverage by sample size.
    double tilt_x{0.0};          ///< Coverage by board tilt along board X.
    double tilt_y{0.0};          ///< Coverage by board tilt along board Y.
    bool data_sufficient{false}; ///< Whether data is sufficient for calibration.
};

}  // namespace clover2_camera_utils::data
