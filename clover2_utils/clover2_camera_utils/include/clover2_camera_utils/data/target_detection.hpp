/**
 * @file target_detection.hpp
 * @brief Provides the result of calibration target detection.
 */

#pragma once

#include <clover2_camera_utils/data/board_orientation.hpp>
#include <clover2_camera_utils/data/observation.hpp>
#include <clover2_camera_utils/data/sample_features.hpp>

#include <opencv2/core/types.hpp>

#include <optional>
#include <vector>

namespace clover2_camera_utils::data {

/**
 * @brief Combines calibration correspondences with target-specific selection
 * features.
 */
struct target_detection {
    observation calibration;       ///< Corresponding image and object points.
    sample_features selection;     ///< Descriptor used by the sample selector.
};

/** @brief Raw target features retained for debug visualization. */
struct target_debug {
    /** Corners of all detected ArUco markers. */
    std::vector<std::vector<cv::Point2f>> marker_corners;
    /** IDs corresponding to @ref marker_corners. */
    std::vector<int> marker_ids;
    /** Detected or interpolated calibration-target corners. */
    std::vector<cv::Point2f> corners;
    /** IDs corresponding to @ref corners when the target provides them. */
    std::vector<int> corner_ids;
    /** Geometry and features of the detected corners when they are valid. */
    std::optional<sample_feature_geometry> feature_geometry;
    /** Homography-based board orientation diagnostics, when available. */
    std::optional<board_orientation> orientation;
};

/**
 * @brief Contains raw visualization data and an optional calibration sample.
 *
 * The debug data is preserved even if the detection does not meet the
 * requirements for inclusion in the calibration dataset.
 */
struct target_detection_result {
    target_debug debug;
    std::optional<target_detection> sample;
};

}  // namespace clover2_camera_utils::data