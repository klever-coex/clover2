#include <clover2_thermal/thermal_frame.hpp>

#include <stdexcept>

namespace clover2_thermal {

thermal_frame::thermal_frame(const cv::Mat& cv_frame) : m_cv_frame(cv_frame) {
    if (cv_frame.empty()) throw std::invalid_argument("Frame is empty");
    if (cv_frame.dims != 2)
        throw std::invalid_argument("Frame is not 2-dimensional");
    if (cv_frame.type() != CV_8UC2)
        throw std::invalid_argument("Frame is not of type CV_8UC2");
    if (cv_frame.rows % 2 != 0)
        throw std::invalid_argument(
            "Frame does not have an even number of rows");
}

cv::Mat thermal_frame::extract_grayscale() const {
    const int h = m_cv_frame.rows / 2;
    const int w = m_cv_frame.cols;

    // Get the upper half of the frame, which contains the pre-processed image
    const cv::Mat half = m_cv_frame(cv::Rect(0, 0, w, h));

    // Extract the first channel (grayscale) from the upper half of the frame
    cv::Mat grayscale;
    cv::extractChannel(half, grayscale, 0);

    return grayscale;
}

cv::Mat thermal_frame::extract_kelvin_temperature() const {
    const int h = m_cv_frame.rows / 2;
    const int w = m_cv_frame.cols;

    // Get the lower half of the frame, which contains the raw 16-bit
    // temperature data
    const cv::Mat half = m_cv_frame(cv::Rect(0, h, w, h));

    // Create a new matrix that shares the same data,
    // but with the correct type (16-bit unsigned)
    const cv::Mat raw16(h, w, CV_16UC1, half.data, half.step);

    // Convert the raw 16-bit temperature data to a 32-bit float matrix
    // and scale it to represent temperature in Kelvin
    cv::Mat temperature;
    raw16.convertTo(temperature, CV_32FC1, 1.0 / 64.0);

    return temperature;
}

}  // namespace clover2_thermal
