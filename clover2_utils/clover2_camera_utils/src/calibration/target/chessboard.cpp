#include <clover2_camera_utils/calibration/target/chessboard.hpp>

#include <clover2_camera_utils/calibration/sample/features.hpp>
#include <clover2_camera_utils/calibration/sample/orientation.hpp>

#include <nlohmann/json.hpp>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace clover2_camera_utils::calibration::target {

std::shared_ptr<const base> chessboard::from_json(
    const nlohmann::json& profile) {
    const auto rows = profile.at("rows").get<uint32_t>();
    const auto columns = profile.at("columns").get<uint32_t>();
    const auto square_size_m = profile.at("square_size_m").get<double>();
    if (rows < 2 || columns < 2 || square_size_m <= 0.0) {
        throw std::invalid_argument(
            "chessboard rows and columns must be at least 2, and "
            "square_size_m must be positive");
    }
    return std::make_shared<chessboard>(rows, columns, square_size_m);
}

chessboard::chessboard(uint32_t rows, uint32_t columns, double square_size_m)
    : m_pattern_size{static_cast<int>(columns), static_cast<int>(rows)} {
    m_object_points.reserve(static_cast<size_t>(rows) * columns);
    for (uint32_t row = 0; row < rows; ++row) {
        for (uint32_t column = 0; column < columns; ++column) {
            m_object_points.emplace_back(column * square_size_m,
                                         row * square_size_m, 0.0F);
        }
    }
}

data::target_detection_result chessboard::detect(
    const cv::Mat& image) const {
    data::target_detection_result result;
    if (image.type() != CV_8UC1) {
        return result;
    }

    std::vector<cv::Point2f> corners;
    const bool found = cv::findChessboardCorners(
        image, m_pattern_size, corners,
        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
    if (!found) {
        return result;
    }

    cv::cornerSubPix(
        image, corners, {11, 11}, {-1, -1},
        {cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 30, 0.1});
    result.debug.corners = corners;
    const std::vector<cv::Point2f> outer_corners{
        corners.front(),
        corners[static_cast<size_t>(m_pattern_size.width - 1)],
        corners.back(),
        corners[corners.size() - static_cast<size_t>(m_pattern_size.width)],
    };
    const auto features =
        sample::features_calculator{}.compute(outer_corners, image.size());
    if (!features.has_value()) {
        return result;
    }
    result.debug.feature_geometry = *features;
    data::observation observation{
        .image_size = image.size(),
        .image_points = std::move(corners),
        .object_points = m_object_points,
    };
    result.debug.orientation = sample::orientation_calculator{}.compute(
        observation.object_points, observation.image_points, m_object_points);
    if (!result.debug.orientation.has_value()) {
        return result;
    }
    auto selection = features->features;
    selection.tilt_x = result.debug.orientation->tilt_x;
    selection.tilt_y = result.debug.orientation->tilt_y;
    result.sample = data::target_detection{
        .calibration = std::move(observation),
        .selection = selection,
    };
    return result;
}

void chessboard::draw_debug(
    cv::Mat& image, const data::target_debug& debug) const {
    cv::drawChessboardCorners(image, m_pattern_size, debug.corners,
                              !debug.corners.empty());
}

}  // namespace clover2_camera_utils::calibration::target
