/**
 * @file orientation.hpp
 * @brief Provides homography-based planar board-orientation diagnostics.
 */

#pragma once

#include <clover2_camera_utils/data/board_orientation.hpp>

#include <opencv2/core/types.hpp>

#include <optional>
#include <vector>

namespace clover2_camera_utils::calibration::sample {

/** @brief Estimates diagnostic orientation proxies from planar correspondences. */
class orientation_calculator {
public:
    /**
     * @brief Estimates a homography with RANSAC and derives board tilt proxies.
     * @param object_points Detected corresponding board-frame points.
     * @param image_points Corresponding image points in pixels.
     * @param full_board_object_points All calibration points on the configured
     * board, used to normalize the tilt proxies.
     * @return Orientation diagnostics, or `std::nullopt` for invalid,
     * degenerate, or non-robust correspondences.
     *
     * The result is intended for diagnostics only. In particular, raw fisheye
     * image coordinates do not obey a projective planar homography exactly.
     */
    std::optional<data::board_orientation> compute(
        const std::vector<cv::Point3f>& object_points,
        const std::vector<cv::Point2f>& image_points,
        const std::vector<cv::Point3f>& full_board_object_points) const;

private:
    static constexpr size_t k_minimum_points = 4;
    static constexpr double k_ransac_reprojection_threshold_px = 2.0;
    static constexpr double k_minimum_span = 1e-6;
    static constexpr double k_minimum_projective_depth = 1e-9;
};

}  // namespace clover2_camera_utils::calibration::sample