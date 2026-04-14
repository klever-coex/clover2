#pragma once

#include <opencv2/core.hpp>

#include <memory>
#include <optional>

namespace clover2::pose::data {

struct camera_intrinsics {
    cv::Mat K;
    cv::Mat dist_coeffs;
};

struct sensor_data {
    double timestamp = 0.0;
    int32_t sensor_id = 0;

    std::optional<cv::Mat> image;
    std::optional<camera_intrinsics> intrinsics;
};

}  // namespace clover2::pose::data
