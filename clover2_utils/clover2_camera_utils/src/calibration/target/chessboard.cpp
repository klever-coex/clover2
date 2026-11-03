#include <clover2_camera_utils/calibration/target/chessboard.hpp>

#include <nlohmann/json.hpp>

#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

#include <fstream>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace clover2_camera_utils::calibration::target {
namespace {

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

using json = nlohmann::json;

}  // namespace

std::shared_ptr<const base> chessboard::from_file(
    const std::filesystem::path& path) {
    if (path.empty() || !path.is_absolute()) {
        throw std::invalid_argument(
            "chessboard configuration path must be absolute");
    }

    std::ifstream input(path);
    if (!input) {
        throw std::runtime_error("could not open chessboard configuration '" +
                                 path.string() + "'");
    }

    const json value = json::parse(input);
    const auto rows = value.at("rows").get<uint32_t>();
    const auto columns = value.at("columns").get<uint32_t>();
    const auto square_size_m = value.at("square_size_m").get<double>();
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

std::optional<data::observation> chessboard::detect(
    const cv::Mat& image) const {
    const auto grayscale = to_grayscale(image);
    if (!grayscale.has_value()) {
        return std::nullopt;
    }

    std::vector<cv::Point2f> corners;
    const bool found = cv::findChessboardCorners(
        *grayscale, m_pattern_size, corners,
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
        .object_points = m_object_points,
    };
}

void chessboard::draw_debug(
    cv::Mat& image, const std::optional<data::observation>& observation) const {
    cv::drawChessboardCorners(
        image, m_pattern_size,
        observation ? observation->image_points : std::vector<cv::Point2f>{},
        observation.has_value());
}

}  // namespace clover2_camera_utils::calibration::target
