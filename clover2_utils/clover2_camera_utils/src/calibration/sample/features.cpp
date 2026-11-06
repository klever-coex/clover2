#include <clover2_camera_utils/calibration/sample/features.hpp>

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>

namespace clover2_camera_utils::calibration::sample {

std::optional<data::sample_features> features_calculator::compute(
    const std::vector<cv::Point2f>& corners, const cv::Size image_size) const {
    if (image_size.width <= 0 || image_size.height <= 0 || corners.size() < 4) {
        return std::nullopt;
    }

    std::vector<cv::Point2f> hull;
    cv::convexHull(corners, hull);
    if (hull.size() < 3) {
        return std::nullopt;
    }

    const double area = std::abs(cv::contourArea(hull));
    if (area <= k_min_area) {
        return std::nullopt;
    }

    cv::Point2d mean;
    for (const auto& corner : corners) {
        mean += cv::Point2d{corner.x, corner.y};
    }
    mean *= 1.0 / static_cast<double>(corners.size());

    const auto skew = calculate_skew(hull);
    if (!skew.has_value()) {
        return std::nullopt;
    }

    const double image_area =
        static_cast<double>(image_size.width) * image_size.height;
    return data::sample_features{
        .x = std::clamp(mean.x / image_size.width, 0.0, 1.0),
        .y = std::clamp(mean.y / image_size.height, 0.0, 1.0),
        .size = std::sqrt(area / image_area),
        .skew = *skew,
    };
}

std::optional<double> features_calculator::calculate_skew(
    const std::vector<cv::Point2f>& hull) {
    const auto rectangle = cv::minAreaRect(hull);
    if (rectangle.size.width <= k_min_area ||
        rectangle.size.height <= k_min_area) {
        return std::nullopt;
    }
    const double rectangle_area =
        static_cast<double>(rectangle.size.width) * rectangle.size.height;
    return std::clamp(1.0 - std::abs(cv::contourArea(hull)) / rectangle_area,
                      0.0, 1.0);
}

}  // namespace clover2_camera_utils::calibration::sample
