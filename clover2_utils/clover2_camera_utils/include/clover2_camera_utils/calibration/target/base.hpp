/**
 * @file base.hpp
 * @brief Provides the base interface for calibration targets.
 */

#pragma once

#include <clover2_camera_utils/data/observation.hpp>

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
     * @brief Detects the target and returns corresponding calibration points.
     * @param image Camera image.
     * @return Observation with detected points or `std::nullopt` if the target
     * is not detected.
     */
    virtual std::optional<data::observation> detect(
        const cv::Mat& image) const = 0;

    /**
     * @brief Draws target detection diagnostics onto an image.
     * @param image Debug image to modify.
     * @param observation Observation returned for the source image.
     */
    virtual void draw_debug(
        cv::Mat& image,
        const std::optional<data::observation>& observation) const = 0;
};

}  // namespace clover2_camera_utils::calibration::target
