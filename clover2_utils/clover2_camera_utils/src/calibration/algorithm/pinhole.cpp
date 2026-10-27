#include <clover2_camera_utils/calibration/algorithm/pinhole.hpp>

#include <opencv2/calib3d.hpp>

#include <vector>

namespace clover2_camera_utils::calibration::algorithm {
namespace {

constexpr size_t k_minimum_observations = 3;

}  // namespace

std::optional<sensor_msgs::msg::CameraInfo>
pinhole::calibrate(
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
        if (!is_valid_observation(observation, image_size)) {
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
        cv::calibrateCamera(object_points, image_points, image_size,
                            camera_matrix, distortion_coefficients,
                            rotation_vectors, translation_vectors);

        return make_camera_info(image_size, camera_matrix,
                                distortion_coefficients, "plumb_bob");
    } catch (const cv::Exception&) {
        return std::nullopt;
    }
}

}  // namespace clover2_camera_utils::calibration::algorithm
