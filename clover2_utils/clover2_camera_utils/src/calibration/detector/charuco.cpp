#include <clover2_camera_utils/calibration/detector/charuco.hpp>

#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>
#include <opencv2/imgproc.hpp>

#include <array>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace clover2_camera_utils::calibration::detector {
namespace {

constexpr size_t k_minimum_corners = 4;

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

bool is_valid(const data::board::charuco_config& board) {
    return board.squares_rows >= 3 && board.squares_columns >= 3 &&
           board.square_size_m > 0.0 && board.marker_size_m > 0.0 &&
           board.marker_size_m < board.square_size_m &&
           k_dictionary_ids.contains(board.dictionary);
}

std::optional<cv::Mat> to_grayscale(const cv::Mat& image) {
    if (image.empty()) {
        return std::nullopt;
    }
    if (image.type() == CV_8UC1) {
        return image;
    }
    if (image.type() == CV_8UC3) {
        cv::Mat grayscale;
        cv::cvtColor(image, grayscale, cv::COLOR_BGR2GRAY);
        return grayscale;
    }
    return std::nullopt;
}

}  // namespace

std::optional<data::observation> charuco::detect(
    const cv::Mat& image, const data::board::config& board) const {
    const auto* config = std::get_if<data::board::charuco_config>(&board);
    if (config == nullptr || !is_valid(*config)) {
        return std::nullopt;
    }
    const auto grayscale = to_grayscale(image);
    if (!grayscale.has_value()) {
        return std::nullopt;
    }

    const auto dictionary = cv::aruco::getPredefinedDictionary(
        k_dictionary_ids.at(config->dictionary));
    const auto charuco_board = cv::aruco::CharucoBoard::create(
        static_cast<int>(config->squares_columns),
        static_cast<int>(config->squares_rows),
        static_cast<float>(config->square_size_m),
        static_cast<float>(config->marker_size_m), dictionary);

    std::vector<std::vector<cv::Point2f>> marker_corners;
    std::vector<int> marker_ids;
    cv::aruco::detectMarkers(*grayscale, dictionary, marker_corners,
                             marker_ids);
    if (marker_ids.empty()) {
        return std::nullopt;
    }

    cv::Mat charuco_corners;
    cv::Mat charuco_ids;
    const auto corner_count = cv::aruco::interpolateCornersCharuco(
        marker_corners, marker_ids, *grayscale, charuco_board, charuco_corners,
        charuco_ids);
    if (corner_count < static_cast<int>(k_minimum_corners)) {
        return std::nullopt;
    }

    data::observation observation;
    observation.image_size = image.size();
    observation.image_points.reserve(static_cast<size_t>(corner_count));
    observation.object_points.reserve(static_cast<size_t>(corner_count));
    for (int index = 0; index < corner_count; ++index) {
        const auto corner_id = charuco_ids.at<int>(index);
        if (corner_id < 0 || static_cast<size_t>(corner_id) >=
                                 charuco_board->chessboardCorners.size()) {
            return std::nullopt;
        }
        observation.image_points.push_back(
            charuco_corners.at<cv::Point2f>(index));
        observation.object_points.push_back(
            charuco_board->chessboardCorners[static_cast<size_t>(corner_id)]);
    }
    return observation;
}

}  // namespace clover2_camera_utils::calibration::detector
