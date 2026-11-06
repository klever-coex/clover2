#include <clover2_camera_utils/calibration/algorithm/base.hpp>

#include <utility>

namespace clover2_camera_utils::calibration::algorithm {

bool base::is_valid_observation(const data::observation& observation,
                                const cv::Size& image_size) {
    constexpr size_t k_minimum_points_per_observation = 4;
    return observation.image_size == image_size &&
           observation.image_points.size() == observation.object_points.size() &&
           observation.image_points.size() >= k_minimum_points_per_observation;
}

sensor_msgs::msg::CameraInfo base::make_camera_info(
    const cv::Size& image_size, const cv::Mat& camera_matrix,
    const cv::Mat& distortion_coefficients, std::string distortion_model) {
    sensor_msgs::msg::CameraInfo camera_info;
    camera_info.width = static_cast<uint32_t>(image_size.width);
    camera_info.height = static_cast<uint32_t>(image_size.height);
    camera_info.distortion_model = std::move(distortion_model);
    for (size_t row = 0; row < 3; ++row) {
        for (size_t column = 0; column < 3; ++column) {
            camera_info.k[row * 3 + column] = camera_matrix.at<double>(
                static_cast<int>(row), static_cast<int>(column));
        }
    }
    camera_info.r[0] = 1.0;
    camera_info.r[4] = 1.0;
    camera_info.r[8] = 1.0;
    camera_info.p[0] = camera_info.k[0];
    camera_info.p[2] = camera_info.k[2];
    camera_info.p[5] = camera_info.k[4];
    camera_info.p[6] = camera_info.k[5];
    camera_info.p[10] = 1.0;
    camera_info.d.reserve(distortion_coefficients.total());
    for (int row = 0; row < distortion_coefficients.rows; ++row) {
        for (int column = 0; column < distortion_coefficients.cols; ++column) {
            camera_info.d.push_back(
                distortion_coefficients.at<double>(row, column));
        }
    }
    return camera_info;
}

}  // namespace clover2_camera_utils::calibration::algorithm
