/**
 * @file board_orientation.hpp
 * @brief Provides homography-based board-orientation diagnostics.
 */

#pragma once

#include <cstddef>

namespace clover2_camera_utils::data {

/**
 * @brief Describes projective orientation of a planar calibration board.
 *
 * The tilt components are signed, dimensionless perspective proxies derived
 * from a homography. They are not physical angles and are only expected to be
 * geometrically accurate for perspective camera images.
 */
struct board_orientation {
    double tilt_x{0.0};                 ///< Perspective change along board X.
    double tilt_y{0.0};                 ///< Perspective change along board Y.
    size_t inlier_count{0};             ///< Correspondences retained by RANSAC.
    double mean_reprojection_error{0.0};  ///< Mean inlier error in pixels.
};

}  // namespace clover2_camera_utils::data