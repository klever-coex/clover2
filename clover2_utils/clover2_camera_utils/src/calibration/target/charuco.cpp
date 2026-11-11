#include <clover2_camera_utils/calibration/target/charuco.hpp>

#include <clover2_camera_utils/calibration/sample/features.hpp>
#include <clover2_camera_utils/calibration/sample/orientation.hpp>

#include <nlohmann/json.hpp>

#include <opencv2/imgproc.hpp>

#include <cmath>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace clover2_camera_utils::calibration::target {
namespace {

constexpr size_t k_minimum_corners = 4;
constexpr double k_minimum_corner_ratio = 0.4;

const std::unordered_map<std::string_view,
                         cv::aruco::PREDEFINED_DICTIONARY_NAME>
    k_dictionary_ids{
        {"DICT_4X4_50", cv::aruco::DICT_4X4_50},
        {"DICT_4X4_100", cv::aruco::DICT_4X4_100},
        {"DICT_4X4_250", cv::aruco::DICT_4X4_250},
        {"DICT_4X4_1000", cv::aruco::DICT_4X4_1000},
        {"DICT_5X5_50", cv::aruco::DICT_5X5_50},
        {"DICT_5X5_100", cv::aruco::DICT_5X5_100},
        {"DICT_5X5_250", cv::aruco::DICT_5X5_250},
        {"DICT_5X5_1000", cv::aruco::DICT_5X5_1000},
        {"DICT_6X6_50", cv::aruco::DICT_6X6_50},
        {"DICT_6X6_100", cv::aruco::DICT_6X6_100},
        {"DICT_6X6_250", cv::aruco::DICT_6X6_250},
        {"DICT_6X6_1000", cv::aruco::DICT_6X6_1000},
        {"DICT_7X7_50", cv::aruco::DICT_7X7_50},
        {"DICT_7X7_100", cv::aruco::DICT_7X7_100},
        {"DICT_7X7_250", cv::aruco::DICT_7X7_250},
        {"DICT_7X7_1000", cv::aruco::DICT_7X7_1000},
        {"DICT_ARUCO_ORIGINAL", cv::aruco::DICT_ARUCO_ORIGINAL},
        {"DICT_APRILTAG_16h5", cv::aruco::DICT_APRILTAG_16h5},
        {"DICT_APRILTAG_25h9", cv::aruco::DICT_APRILTAG_25h9},
        {"DICT_APRILTAG_36h10", cv::aruco::DICT_APRILTAG_36h10},
        {"DICT_APRILTAG_36h11", cv::aruco::DICT_APRILTAG_36h11},
    };

}  // namespace

std::shared_ptr<const base> charuco::from_json(
    const nlohmann::json& profile) {
    const auto squares_rows = profile.at("squares_rows").get<uint32_t>();
    const auto squares_columns =
        profile.at("squares_columns").get<uint32_t>();
    const auto square_size_m = profile.at("square_size_m").get<double>();
    const auto marker_size_m = profile.at("marker_size_m").get<double>();
    const auto dictionary = profile.at("dictionary").get<std::string>();
    if (squares_rows < 3 || squares_columns < 3 || square_size_m <= 0.0 ||
        marker_size_m <= 0.0 || marker_size_m >= square_size_m) {
        throw std::invalid_argument(
            "charuco squares_rows and squares_columns must be at least 3, "
            "and marker_size_m must be positive and smaller than "
            "square_size_m");
    }
    return std::make_shared<charuco>(squares_rows, squares_columns,
                                     square_size_m, marker_size_m, dictionary);
}

charuco::charuco(uint32_t squares_rows, uint32_t squares_columns,
                 double square_size_m, double marker_size_m,
                 std::string dictionary) {
    const auto dictionary_it = k_dictionary_ids.find(dictionary);
    if (dictionary_it == k_dictionary_ids.end()) {
        throw std::invalid_argument("unsupported ArUco dictionary '" +
                                    dictionary + "'");
    }

    m_dictionary = cv::aruco::getPredefinedDictionary(dictionary_it->second);
    m_board = cv::aruco::CharucoBoard::create(
        static_cast<int>(squares_columns), static_cast<int>(squares_rows),
        static_cast<float>(square_size_m), static_cast<float>(marker_size_m),
        m_dictionary);
}

data::target_detection_result charuco::detect(
    const cv::Mat& image) const {
    data::target_detection_result result;
    if (image.type() != CV_8UC1) {
        return result;
    }

    cv::aruco::detectMarkers(image, m_dictionary, result.debug.marker_corners,
                             result.debug.marker_ids);
    if (result.debug.marker_ids.empty()) {
        return result;
    }

    cv::Mat charuco_corners;
    cv::Mat charuco_ids;
    const auto corner_count = cv::aruco::interpolateCornersCharuco(
        result.debug.marker_corners, result.debug.marker_ids, image, m_board,
        charuco_corners, charuco_ids);
    result.debug.corners.reserve(static_cast<size_t>(corner_count));
    result.debug.corner_ids.reserve(static_cast<size_t>(corner_count));
    for (int index = 0; index < corner_count; ++index) {
        result.debug.corners.push_back(
            charuco_corners.at<cv::Point2f>(index));
        result.debug.corner_ids.push_back(charuco_ids.at<int>(index));
    }

    if (corner_count >= static_cast<int>(k_minimum_corners)) {
        result.debug.feature_geometry = sample::features_calculator{}.compute(
            result.debug.corners, image.size());
    }

    const auto minimum_corner_count = static_cast<int>(std::ceil(
        k_minimum_corner_ratio * m_board->chessboardCorners.size()));
    if (corner_count < static_cast<int>(k_minimum_corners) ||
        corner_count < minimum_corner_count) {
        return result;
    }

    data::observation observation;
    observation.image_size = image.size();
    observation.image_points.reserve(static_cast<size_t>(corner_count));
    observation.object_points.reserve(static_cast<size_t>(corner_count));
    for (int index = 0; index < corner_count; ++index) {
        const auto corner_id = charuco_ids.at<int>(index);
        if (corner_id < 0 || static_cast<size_t>(corner_id) >=
                                 m_board->chessboardCorners.size()) {
            return result;
        }
        observation.image_points.push_back(
            charuco_corners.at<cv::Point2f>(index));
        observation.object_points.push_back(
            m_board->chessboardCorners[static_cast<size_t>(corner_id)]);
    }
    if (!result.debug.feature_geometry.has_value()) {
        return result;
    }
    result.debug.orientation = sample::orientation_calculator{}.compute(
        observation.object_points, observation.image_points,
        m_board->chessboardCorners);
    if (!result.debug.orientation.has_value()) {
        return result;
    }
    auto selection = result.debug.feature_geometry->features;
    selection.tilt_x = result.debug.orientation->tilt_x;
    selection.tilt_y = result.debug.orientation->tilt_y;
    result.sample = data::target_detection{
        .calibration = std::move(observation),
        .selection = selection,
    };
    return result;
}

void charuco::draw_debug(
    cv::Mat& image, const data::target_debug& debug) const {
    if (!debug.marker_ids.empty()) {
        cv::aruco::drawDetectedMarkers(image, debug.marker_corners,
                                       debug.marker_ids);
    }
    for (size_t index = 0; index < debug.corners.size(); ++index) {
        const auto& point = debug.corners[index];
        cv::circle(image, point, 6, cv::Scalar{0, 0, 0}, 2, cv::LINE_AA);
        cv::circle(image, point, 3, cv::Scalar{0, 255, 0}, cv::FILLED,
                   cv::LINE_AA);
        if (index < debug.corner_ids.size()) {
            cv::putText(image, std::to_string(debug.corner_ids[index]),
                        point + cv::Point2f{7.0F, -7.0F},
                        cv::FONT_HERSHEY_SIMPLEX, 0.45, cv::Scalar{0, 255, 0},
                        1, cv::LINE_AA);
        }
    }
    if (!debug.feature_geometry.has_value()) {
        return;
    }

    const auto& geometry = *debug.feature_geometry;
    for (size_t index = 0; index < geometry.convex_hull.size(); ++index) {
        cv::line(image, geometry.convex_hull[index],
                 geometry.convex_hull[(index + 1) % geometry.convex_hull.size()],
                 cv::Scalar{0, 255, 255}, 2, cv::LINE_AA);
    }
    cv::drawMarker(image, geometry.mean, cv::Scalar{0, 0, 255},
                   cv::MARKER_CROSS, 16, 2, cv::LINE_AA);

    const auto& features = geometry.features;
    const std::string text = cv::format(
        "x=%.3f y=%.3f size=%.3f", features.x, features.y, features.size);
    cv::putText(image, text, {16, 30}, cv::FONT_HERSHEY_SIMPLEX, 0.65,
                cv::Scalar{0, 165, 255}, 2, cv::LINE_AA);
}

}  // namespace clover2_camera_utils::calibration::target
