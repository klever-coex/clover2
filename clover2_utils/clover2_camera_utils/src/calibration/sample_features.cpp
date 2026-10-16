#include <clover2_camera_utils/calibration/sample_features.hpp>

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace clover2_camera_utils::calibration {

std::optional<data::sample_features> sample_features_calculator::compute(
    const std::vector<cv::Point2f>& corners, const data::board_config& board,
    const cv::Size image_size) const {
    if (image_size.width <= 0 || image_size.height <= 0 || board.rows < 2 ||
        board.columns < 2) {
        return std::nullopt;
    }

    const auto expected_corner_count =
        static_cast<size_t>(board.rows) * static_cast<size_t>(board.columns);
    if (corners.size() != expected_corner_count) {
        return std::nullopt;
    }

    const std::array<cv::Point2f, 4> outer_corners{
        corners.front(),
        corners[board.columns - 1],
        corners.back(),
        corners[corners.size() - board.columns],
    };

    const double area = calculate_area(outer_corners);
    if (area <= k_min_area) {
        return std::nullopt;
    }

    cv::Point2d mean;
    for (const auto& corner : corners) {
        mean += cv::Point2d{corner.x, corner.y};
    }
    mean *= 1.0 / static_cast<double>(corners.size());

    const double border = std::sqrt(area);
    const double available_width = image_size.width - border;
    const double available_height = image_size.height - border;
    if (available_width <= 0.0 || available_height <= 0.0) {
        return std::nullopt;
    }

    const auto skew = calculate_skew(outer_corners);
    if (!skew.has_value()) {
        return std::nullopt;
    }

    const double image_area =
        static_cast<double>(image_size.width) * image_size.height;
    return data::sample_features{
        .x = std::clamp((mean.x - border / 2.0) / available_width, 0.0, 1.0),
        .y = std::clamp((mean.y - border / 2.0) / available_height, 0.0, 1.0),
        .size = std::sqrt(area / image_area),
        .skew = *skew,
    };
}

double sample_features_calculator::calculate_area(
    const std::array<cv::Point2f, 4>& corners) {
    const auto& up_left = corners[0];
    const auto& up_right = corners[1];
    const auto& down_right = corners[2];
    const auto& down_left = corners[3];

    const cv::Point2f a = up_right - up_left;
    const cv::Point2f b = down_right - up_right;
    const cv::Point2f c = down_left - down_right;
    const cv::Point2f p = b + c;
    const cv::Point2f q = a + b;
    return std::abs(static_cast<double>(p.x) * q.y -
                    static_cast<double>(p.y) * q.x) /
           2.0;
}

std::optional<double> sample_features_calculator::calculate_skew(
    const std::array<cv::Point2f, 4>& corners) {
    const auto& up_left = corners[0];
    const auto& up_right = corners[1];
    const auto& down_right = corners[2];

    const cv::Point2f left_edge = up_left - up_right;
    const cv::Point2f right_edge = down_right - up_right;
    const double lengths = cv::norm(left_edge) * cv::norm(right_edge);
    if (lengths <= k_min_area) {
        return std::nullopt;
    }

    const double cosine = std::clamp(
        static_cast<double>(left_edge.dot(right_edge)) / lengths, -1.0, 1.0);
    const double angle = std::acos(cosine);
    return std::min(1.0, 2.0 * std::abs(std::numbers::pi / 2.0 - angle));
}

}  // namespace clover2_camera_utils::calibration
