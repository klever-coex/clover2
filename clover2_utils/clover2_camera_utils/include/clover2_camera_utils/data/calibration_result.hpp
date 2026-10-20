#pragma once

#include <opencv2/core/mat.hpp>

#include <vector>

namespace clover2_camera_utils::data {

struct calibration_result {
    cv::Size image_size;
    cv::Mat camera_matrix;
    cv::Mat distortion_coefficients;
    std::vector<cv::Mat> rotation_vectors;
    std::vector<cv::Mat> translation_vectors;
};

}  // namespace clover2_camera_utils::data
