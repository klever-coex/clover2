#include <clover2_camera_utils/calibration/sample/orientation.hpp>

#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace clover2_camera_utils::calibration::sample {

std::optional<data::board_orientation> orientation_calculator::compute(
    const std::vector<cv::Point3f>& object_points,
    const std::vector<cv::Point2f>& image_points,
    const std::vector<cv::Point3f>& full_board_object_points) const {
    if (object_points.size() != image_points.size() ||
        object_points.size() < k_minimum_points ||
        full_board_object_points.size() < k_minimum_points) {
        return std::nullopt;
    }

    std::vector<cv::Point2f> board_points;
    board_points.reserve(object_points.size());
    for (const auto& point : object_points) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y) ||
            !std::isfinite(point.z)) {
            return std::nullopt;
        }
        board_points.emplace_back(point.x, point.y);
    }

    double minimum_x = std::numeric_limits<double>::infinity();
    double maximum_x = -std::numeric_limits<double>::infinity();
    double minimum_y = std::numeric_limits<double>::infinity();
    double maximum_y = -std::numeric_limits<double>::infinity();
    for (const auto& point : full_board_object_points) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y) ||
            !std::isfinite(point.z)) {
            return std::nullopt;
        }
        minimum_x = std::min(minimum_x, static_cast<double>(point.x));
        maximum_x = std::max(maximum_x, static_cast<double>(point.x));
        minimum_y = std::min(minimum_y, static_cast<double>(point.y));
        maximum_y = std::max(maximum_y, static_cast<double>(point.y));
    }
    for (const auto& point : image_points) {
        if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
            return std::nullopt;
        }
    }

    const double span_x = maximum_x - minimum_x;
    const double span_y = maximum_y - minimum_y;
    if (span_x <= k_minimum_span || span_y <= k_minimum_span) {
        return std::nullopt;
    }

    cv::Mat inlier_mask;
    cv::Mat homography;
    try {
        homography = cv::findHomography(
            board_points, image_points, cv::RANSAC,
            k_ransac_reprojection_threshold_px, inlier_mask);
    } catch (const cv::Exception&) {
        return std::nullopt;
    }
    if (homography.empty() || homography.rows != 3 || homography.cols != 3 ||
        inlier_mask.total() != board_points.size()) {
        return std::nullopt;
    }

    cv::Mat homography64;
    homography.convertTo(homography64, CV_64F);
    const auto h = [&homography64](int row, int column) {
        return homography64.at<double>(row, column);
    };
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            if (!std::isfinite(h(row, column))) {
                return std::nullopt;
            }
        }
    }

    std::vector<cv::Point2f> projected_points;
    cv::perspectiveTransform(board_points, projected_points, homography64);
    double total_error = 0.0;
    size_t inlier_count = 0;
    for (size_t index = 0; index < board_points.size(); ++index) {
        if (inlier_mask.at<uchar>(static_cast<int>(index)) == 0) {
            continue;
        }
        const auto error = cv::norm(projected_points[index] - image_points[index]);
        if (!std::isfinite(error)) {
            return std::nullopt;
        }
        total_error += error;
        ++inlier_count;
    }
    if (inlier_count < k_minimum_points) {
        return std::nullopt;
    }

    const double center_x = (minimum_x + maximum_x) * 0.5;
    const double center_y = (minimum_y + maximum_y) * 0.5;
    const double center_depth = h(2, 0) * center_x + h(2, 1) * center_y + h(2, 2);
    if (!std::isfinite(center_depth) ||
        std::abs(center_depth) <= k_minimum_projective_depth) {
        return std::nullopt;
    }

    const double tilt_x = h(2, 0) * span_x / center_depth;
    const double tilt_y = h(2, 1) * span_y / center_depth;
    if (!std::isfinite(tilt_x) || !std::isfinite(tilt_y)) {
        return std::nullopt;
    }

    return data::board_orientation{
        .tilt_x = tilt_x,
        .tilt_y = tilt_y,
        .inlier_count = inlier_count,
        .mean_reprojection_error = total_error / static_cast<double>(inlier_count),
    };
}

}  // namespace clover2_camera_utils::calibration::sample