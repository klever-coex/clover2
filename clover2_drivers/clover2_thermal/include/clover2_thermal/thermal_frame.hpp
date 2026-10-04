#pragma once

#include <opencv2/core.hpp>

namespace clover2_thermal {

class thermal_frame {
public:
    // Validate a complete CV_8UC2 camera frame; throws std::invalid_argument.
    // Shares the frame data, which must not be modified during extraction.
    explicit thermal_frame(const cv::Mat& cv_frame);

    // Extract an owned grayscale image (CV_8UC1) from the upper half.
    cv::Mat extract_grayscale() const;

    // Convert the lower half to owned temperature data in Kelvin (CV_32FC1).
    cv::Mat extract_kelvin_temperature() const;


private:
    cv::Mat m_cv_frame;
};

}  // namespace clover2_thermal
