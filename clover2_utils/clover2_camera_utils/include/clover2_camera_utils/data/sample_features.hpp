/**
 * @file sample_features.hpp
 * @brief Provides sample features for camera calibration.
 */

#pragma once

namespace clover2_camera_utils::data {

/** @brief Stores normalized sample features. */
struct sample_features {
    double x{0.0};     ///< Horizontal sample position.
    double y{0.0};     ///< Vertical sample position.
    double size{0.0};  ///< Relative sample size.
    double skew{0.0};  ///< Sample skew.
};

}  // namespace clover2_camera_utils::data
