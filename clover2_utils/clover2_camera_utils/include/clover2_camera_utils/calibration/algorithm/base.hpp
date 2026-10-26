/**
 * @file base.hpp
 * @brief Provides the base algorithm interface for camera calibration.
 */

#pragma once

#include <clover2_camera_utils/data/observation.hpp>

#include <opencv2/core/mat.hpp>
#include <sensor_msgs/msg/camera_info.hpp>

#include <optional>
#include <string>
#include <vector>

namespace clover2_camera_utils::calibration::algorithm {

/** @brief Defines the algorithm interface for camera calibration. */
class base {
public:
    virtual ~base() = default;

    /**
     * @brief Performs calibration using observations.
     * @param observations Calibration board observations.
     * @return Camera information or `std::nullopt` if calculation is not
     * possible.
     */
    virtual std::optional<sensor_msgs::msg::CameraInfo> calibrate(
        const std::vector<data::observation>& observations) const = 0;

protected:
    /**
     * @brief Creates camera information from OpenCV calibration output.
     * @param image_size Calibrated image size.
     * @param camera_matrix Camera matrix.
     * @param distortion_coefficients Distortion coefficients.
     * @param distortion_model ROS distortion model identifier.
     * @return Camera information populated with calibration parameters.
     */
    static sensor_msgs::msg::CameraInfo make_camera_info(
        const cv::Size& image_size, const cv::Mat& camera_matrix,
        const cv::Mat& distortion_coefficients, std::string distortion_model);
};

}  // namespace clover2_camera_utils::calibration::algorithm
