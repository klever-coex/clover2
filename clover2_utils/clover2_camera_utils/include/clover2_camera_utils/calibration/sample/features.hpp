/**
 * @file features.hpp
 * @brief Provides a sample feature calculator.
 */

#pragma once

#include <clover2_camera_utils/data/board_config.hpp>
#include <clover2_camera_utils/data/sample_features.hpp>

#include <opencv2/core/types.hpp>

#include <array>
#include <optional>
#include <vector>

namespace clover2_camera_utils::calibration::sample {

/** @brief Computes sample features from calibration board corners. */
class features_calculator {
public:
    /**
     * @brief Computes sample position, size, and skew.
     * @param corners Detected calibration board corners.
     * @param board Calibration board configuration.
     * @param image_size Image size in pixels.
     * @return Computed sample features or `std::nullopt` if input data is
     * invalid.
     */
    std::optional<data::sample_features> compute(
        const std::vector<cv::Point2f>& corners,
        const data::board_config& board, cv::Size image_size) const;

private:
    static constexpr double k_min_area = 1e-6;

    static double calculate_area(const std::array<cv::Point2f, 4>& corners);
    static std::optional<double> calculate_skew(
        const std::array<cv::Point2f, 4>& corners);
};

}  // namespace clover2_camera_utils::calibration::sample
