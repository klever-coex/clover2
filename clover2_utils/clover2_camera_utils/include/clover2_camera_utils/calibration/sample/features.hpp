/**
 * @file features.hpp
 * @brief Provides a sample feature calculator.
 */

#pragma once

#include <clover2_camera_utils/data/sample_features.hpp>

#include <opencv2/core/types.hpp>

#include <optional>
#include <vector>

namespace clover2_camera_utils::calibration::sample {

/** @brief Computes sample features from calibration board corners. */
class features_calculator {
public:
    /**
     * @brief Computes sample position and size.
     * @param corners Detected calibration board corners.
     * @param image_size Image size in pixels.
     * @return Computed features together with their source geometry, or
     * `std::nullopt` if input data is invalid.
     */
    std::optional<data::sample_feature_geometry> compute(
        const std::vector<cv::Point2f>& corners, cv::Size image_size) const;

private:
    static constexpr double k_min_area = 1e-6;
};

}  // namespace clover2_camera_utils::calibration::sample
