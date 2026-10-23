/**
 * @file calibration_result.hpp
 * @brief Provides a camera calibration result.
 */

#pragma once

#include <opencv2/core/mat.hpp>

#include <string>
#include <vector>

namespace clover2_camera_utils::data {

/** @brief Stores parameters calculated during camera calibration. */
struct calibration_result {
    cv::Size image_size;  ///< Size of images used for calibration.
    cv::Mat camera_matrix;            ///< Camera matrix.
    cv::Mat distortion_coefficients;  ///< Distortion coefficients.
    std::vector<cv::Mat>
        rotation_vectors;  ///< Rotation vectors for observations.
    std::vector<cv::Mat>
        translation_vectors;  ///< Translation vectors for observations.
    std::string distortion_model{"plumb_bob"};  ///< ROS distortion model identifier.
};

}  // namespace clover2_camera_utils::data
