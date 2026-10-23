/**
 * @file base.hpp
 * @brief Provides the base detector interface for calibration boards.
 */

#pragma once

#include <clover2_camera_utils/data/board_config.hpp>
#include <clover2_camera_utils/data/observation.hpp>

#include <opencv2/core/mat.hpp>

#include <optional>

namespace clover2_camera_utils::calibration::detector {

/** @brief Defines the detector interface for calibration boards. */
class base {
public:
    virtual ~base() = default;

    /**
     * @brief Detects a board in an image.
     * @param image Camera image.
     * @param board Calibration board configuration.
     * @return Observation with detected points or `std::nullopt` if the board
     * is not detected.
     */
    virtual std::optional<data::observation> detect(
        const cv::Mat& image, const data::board_config& board) const = 0;
};

}  // namespace clover2_camera_utils::calibration::detector
