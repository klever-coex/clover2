#include <clover2_camera_utils/calibration/target/charuco.hpp>

#include <clover2_camera_utils/calibration/sample/features.hpp>

#include <nlohmann/json.hpp>

#include <opencv2/imgproc.hpp>

#include <cmath>
#include <optional>
#include <stdexcept>
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

std::optional<data::target_detection> charuco::detect(
    const cv::Mat& image) const {
    if (image.type() != CV_8UC1) {
        return std::nullopt;
    }

    std::vector<std::vector<cv::Point2f>> marker_corners;
    std::vector<int> marker_ids;
    cv::aruco::detectMarkers(image, m_dictionary, marker_corners,
                             marker_ids);
    if (marker_ids.empty()) {
        return std::nullopt;
    }

    cv::Mat charuco_corners;
    cv::Mat charuco_ids;
    const auto corner_count = cv::aruco::interpolateCornersCharuco(
        marker_corners, marker_ids, image, m_board, charuco_corners,
        charuco_ids);
    const auto minimum_corner_count = static_cast<int>(std::ceil(
        k_minimum_corner_ratio * m_board->chessboardCorners.size()));
    if (corner_count < static_cast<int>(k_minimum_corners) ||
        corner_count < minimum_corner_count) {
        return std::nullopt;
    }

    data::observation observation;
    observation.image_size = image.size();
    observation.image_points.reserve(static_cast<size_t>(corner_count));
    observation.object_points.reserve(static_cast<size_t>(corner_count));
    for (int index = 0; index < corner_count; ++index) {
        const auto corner_id = charuco_ids.at<int>(index);
        if (corner_id < 0 || static_cast<size_t>(corner_id) >=
                                 m_board->chessboardCorners.size()) {
            return std::nullopt;
        }
        observation.image_points.push_back(
            charuco_corners.at<cv::Point2f>(index));
        observation.object_points.push_back(
            m_board->chessboardCorners[static_cast<size_t>(corner_id)]);
    }
    const auto features = sample::features_calculator{}.compute(
        observation.image_points, observation.image_size);
    if (!features.has_value()) {
        return std::nullopt;
    }
    return data::target_detection{
        .calibration = std::move(observation),
        .selection = *features,
    };
}

void charuco::draw_debug(
    cv::Mat& image, const std::optional<data::target_detection>& sample) const {
    if (!sample) {
        return;
    }
    for (const auto& point : sample->calibration.image_points) {
        cv::circle(image, point, 6, cv::Scalar{0, 0, 0}, 2, cv::LINE_AA);
        cv::circle(image, point, 3, cv::Scalar{0, 255, 0}, cv::FILLED,
                   cv::LINE_AA);
    }
}

}  // namespace clover2_camera_utils::calibration::target
