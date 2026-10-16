#include <clover2_camera_utils/calibration/chessboard_detector.hpp>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>

namespace clover2_camera_utils::calibration {
namespace {

bool is_valid(const data::board_config& board) {
    return board.rows >= 2 && board.columns >= 2 && board.square_size_m > 0.0;
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

std::vector<cv::Point3f> make_object_points(const data::board_config& board) {
    std::vector<cv::Point3f> points;
    points.reserve(static_cast<size_t>(board.rows) * board.columns);

    for (uint32_t row = 0; row < board.rows; ++row) {
        for (uint32_t column = 0; column < board.columns; ++column) {
            points.emplace_back(column * board.square_size_m,
                                row * board.square_size_m, 0.0F);
        }
    }
    return points;
}

}  // namespace

std::optional<data::observation> chessboard_detector::detect(
    const cv::Mat& image, const data::board_config& board) const {
    if (!is_valid(board)) {
        return std::nullopt;
    }

    const auto grayscale = to_grayscale(image);
    if (!grayscale.has_value()) {
        return std::nullopt;
    }

    const cv::Size pattern_size{static_cast<int>(board.columns),
                                static_cast<int>(board.rows)};
    std::vector<cv::Point2f> corners;
    const bool found = cv::findChessboardCorners(
        *grayscale, pattern_size, corners,
        cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE);
    if (!found) {
        return std::nullopt;
    }

    cv::cornerSubPix(
        *grayscale, corners, {11, 11}, {-1, -1},
        {cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 30, 0.1});

    return data::observation{
        .image_size = image.size(),
        .image_points = std::move(corners),
        .object_points = make_object_points(board),
    };
}

}  // namespace clover2_camera_utils::calibration
