#include <clover2_camera_utils/calibration/pinhole_calibration_algorithm.hpp>

#include <opencv2/calib3d.hpp>

#include <vector>

namespace clover2_camera_utils::calibration {
namespace {

constexpr size_t k_minimum_observations = 3;
constexpr size_t k_minimum_points_per_observation = 4;

bool is_valid(const data::observation& observation, const cv::Size image_size) {
    return observation.image_size == image_size &&
           observation.image_points.size() ==
               observation.object_points.size() &&
           observation.image_points.size() >= k_minimum_points_per_observation;
}

}  // namespace

std::optional<data::calibration_result>
pinhole_calibration_algorithm::calibrate(
    const std::vector<data::observation>& observations) const {
    if (observations.size() < k_minimum_observations) {
        return std::nullopt;
    }

    const cv::Size image_size = observations.front().image_size;
    if (image_size.width <= 0 || image_size.height <= 0) {
        return std::nullopt;
    }

    std::vector<std::vector<cv::Point3f>> object_points;
    std::vector<std::vector<cv::Point2f>> image_points;
    object_points.reserve(observations.size());
    image_points.reserve(observations.size());

    for (const auto& observation : observations) {
        if (!is_valid(observation, image_size)) {
            return std::nullopt;
        }
        object_points.push_back(observation.object_points);
        image_points.push_back(observation.image_points);
    }

    cv::Mat camera_matrix;
    cv::Mat distortion_coefficients;
    std::vector<cv::Mat> rotation_vectors;
    std::vector<cv::Mat> translation_vectors;

    try {
        const double rms = cv::calibrateCamera(
            object_points, image_points, image_size, camera_matrix,
            distortion_coefficients, rotation_vectors, translation_vectors);
        return data::calibration_result{
            .image_size = image_size,
            .camera_matrix = std::move(camera_matrix),
            .distortion_coefficients = std::move(distortion_coefficients),
            .rms_reprojection_error = rms,
            .rotation_vectors = std::move(rotation_vectors),
            .translation_vectors = std::move(translation_vectors),
        };
    } catch (const cv::Exception&) {
        return std::nullopt;
    }
}

}  // namespace clover2_camera_utils::calibration
