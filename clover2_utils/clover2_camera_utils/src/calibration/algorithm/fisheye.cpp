#include <clover2_camera_utils/calibration/algorithm/fisheye.hpp>

#include <opencv2/calib3d.hpp>

#include <vector>

namespace clover2_camera_utils::calibration::algorithm {
namespace {

constexpr size_t k_minimum_observations = 3;

}  // namespace

std::optional<result> fisheye::calibrate(
    const std::vector<data::observation>& observations,
    const std::optional<sensor_msgs::msg::CameraInfo>& initial_estimate) const {
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

    cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distortion_coefficients = cv::Mat::zeros(4, 1, CV_64F);
    std::vector<cv::Mat> rotation_vectors;
    std::vector<cv::Mat> translation_vectors;

    int flags = cv::fisheye::CALIB_RECOMPUTE_EXTRINSIC;
    if (initial_estimate) {
        const auto initial_camera_matrix =
            camera_matrix_from(*initial_estimate);
        const auto initial_distortion_coefficients =
            distortion_coefficients_from(*initial_estimate, 4);
        if (!initial_camera_matrix || !initial_distortion_coefficients) {
            return std::nullopt;
        }
        camera_matrix = *initial_camera_matrix;
        distortion_coefficients = *initial_distortion_coefficients;
        flags |= cv::fisheye::CALIB_USE_INTRINSIC_GUESS;
    }

    try {
        const double rms_reprojection_error = cv::fisheye::calibrate(
            object_points, image_points, image_size, camera_matrix,
            distortion_coefficients, rotation_vectors, translation_vectors,
            flags);

        return result{make_camera_info(image_size, camera_matrix,
                                       distortion_coefficients, "equidistant"),
                      rms_reprojection_error};
    } catch (const cv::Exception&) {
        return std::nullopt;
    }
}

std::optional<std::vector<cv::Point2f>> fisheye::undistort_points(
    const std::vector<cv::Point2f>& image_points,
    const sensor_msgs::msg::CameraInfo& camera_info) const {
    if (image_points.empty()) {
        return std::nullopt;
    }
    const auto camera_matrix = camera_matrix_from(camera_info);
    const auto distortion_coefficients =
        distortion_coefficients_from(camera_info, 4);
    if (!camera_matrix || !distortion_coefficients) {
        return std::nullopt;
    }
    try {
        std::vector<cv::Point2f> undistorted_points;
        cv::fisheye::undistortPoints(image_points, undistorted_points,
                                     *camera_matrix, *distortion_coefficients,
                                     cv::noArray(), *camera_matrix);
        return undistorted_points;
    } catch (const cv::Exception&) {
        return std::nullopt;
    }
}

}  // namespace clover2_camera_utils::calibration::algorithm
