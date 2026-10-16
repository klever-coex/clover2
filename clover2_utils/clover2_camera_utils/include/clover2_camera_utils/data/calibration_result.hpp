#pragma once

#include <opencv2/core/mat.hpp>

#include <vector>

namespace clover2_camera_utils::data {

struct calibration_result {
    cv::Size image_size;
    cv::Mat camera_matrix;
    cv::Mat distortion_coefficients;
    double rms_reprojection_error{0.0};
    std::vector<cv::Mat> rotation_vectors;
    std::vector<cv::Mat> translation_vectors;
};

}  // namespace clover2_camera_utils::data
