/**
 * @file sample_features.hpp
 * @brief Provides sample features for camera calibration.
 */

#pragma once

#include <opencv2/core/types.hpp>

#include <vector>

namespace clover2_camera_utils::data {

/** @brief Stores normalized sample features. */
struct sample_features {
    double x{0.0};     ///< Horizontal sample position.
    double y{0.0};     ///< Vertical sample position.
    double size{0.0};  ///< Relative sample size.
    double tilt_x{0.0};  ///< Homography-based board tilt along board X.
    double tilt_y{0.0};  ///< Homography-based board tilt along board Y.
};

/** @brief Geometry used to calculate and visualize sample features. */
struct sample_feature_geometry {
    sample_features features;                 ///< Calculated normalized features.
    std::vector<cv::Point2f> convex_hull;     ///< Convex hull of input corners.
    cv::Point2f mean;                         ///< Mean of input corners.
};

}  // namespace clover2_camera_utils::data
