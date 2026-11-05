/**
 * @file base.hpp
 * @brief Provides the base interface for calibration targets.
 */

#pragma once

#include <clover2_camera_utils/data/target_detection.hpp>

#include <opencv2/core/mat.hpp>

#include <optional>

namespace clover2_camera_utils::calibration::target {

/**
 * @brief Represents an immutable calibration target prepared for one session.
 */
class base {
public:
    virtual ~base() = default;

    /**
     * @brief Detects the target and describes the resulting sample.
     * @param image Grayscale camera image in `CV_8UC1` format.
     * @return Calibration correspondences and selection features, or
     * `std::nullopt` if the target is not detected.
     */
    virtual std::optional<data::target_detection> detect(
        const cv::Mat& image) const = 0;

    /**
     * @brief Draws target detection diagnostics onto an image.
     * @param image BGR debug image in `CV_8UC3` format to modify.
     * @param sample Sample returned for the source image.
     */
    virtual void draw_debug(
        cv::Mat& image,
        const std::optional<data::target_detection>& sample) const = 0;
};

}  // namespace clover2_camera_utils::calibration::target
