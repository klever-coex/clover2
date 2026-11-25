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

/** @brief Stores the output of a camera calibration algorithm. */
struct result {
    sensor_msgs::msg::CameraInfo camera_info;
    double rms_reprojection_error{};
};

/** @brief Defines the algorithm interface for camera calibration. */
class base {
public:
    virtual ~base() = default;

    /**
     * @brief Performs calibration using observations.
     * @param observations Calibration board observations.
     * @return Camera information and RMS reprojection error in pixels, or
     * `std::nullopt` if calculation is not possible.
     */
    virtual std::optional<result> calibrate(
        const std::vector<data::observation>& observations,
        const std::optional<sensor_msgs::msg::CameraInfo>& initial_estimate =
            std::nullopt) const = 0;

    /**
     * @brief Removes distortion from image points for this camera model.
     * @param image_points Distorted image coordinates in pixels.
     * @param camera_info Camera parameters describing the distortion.
     * @return Undistorted image coordinates in pixels, or `std::nullopt` if
     * the parameters or points are invalid.
     */
    virtual std::optional<std::vector<cv::Point2f>> undistort_points(
        const std::vector<cv::Point2f>& image_points,
        const sensor_msgs::msg::CameraInfo& camera_info) const = 0;

protected:
    /**
     * @brief Checks whether an observation is suitable for calibration.
     * @param observation Calibration board observation.
     * @param image_size Expected image size.
     * @return `true` if image and object points are paired, sufficiently
     * numerous, and match the expected image size.
     */
    static bool is_valid_observation(const data::observation& observation,
                                     const cv::Size& image_size);

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

    /** @brief Converts validated ROS camera intrinsics to an OpenCV matrix. */
    static std::optional<cv::Mat> camera_matrix_from(
        const sensor_msgs::msg::CameraInfo& camera_info);

    /** @brief Converts validated ROS distortion coefficients to OpenCV form. */
    static std::optional<cv::Mat> distortion_coefficients_from(
        const sensor_msgs::msg::CameraInfo& camera_info, size_t expected_count);
};

}  // namespace clover2_camera_utils::calibration::algorithm
