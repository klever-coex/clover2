#pragma once

#include <clover2_camera_utils/data/board_config.hpp>
#include <clover2_camera_utils/data/sample_features.hpp>

#include <opencv2/core/types.hpp>

#include <array>
#include <optional>
#include <vector>

namespace clover2_camera_utils::calibration::sample {

class features_calculator {
public:
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
