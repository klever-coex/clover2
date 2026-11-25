/**
 * @file feature_values.hpp
 * @brief Provides named values for camera calibration sample features.
 */

#pragma once

namespace clover2_camera_utils::data {

/** @brief Stores normalized values for sample features. */
struct feature_values {
    double x{0.0};       ///< Horizontal sample position.
    double y{0.0};       ///< Vertical sample position.
    double size{0.0};    ///< Relative sample size.
    double tilt_x{0.0};  ///< Homography-based board tilt along board X.
    double tilt_y{0.0};  ///< Homography-based board tilt along board Y.
};

}  // namespace clover2_camera_utils::data
