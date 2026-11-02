/**
 * @file charuco.hpp
 * @brief Provides a detector for ChArUco calibration boards.
 */

#pragma once

#include <clover2_camera_utils/calibration/detector/base.hpp>

#include <optional>

namespace clover2_camera_utils::calibration::detector {

/** @brief Detects interpolated ChArUco corners. */
class charuco final : public base {
public:
    static constexpr const char* name = "charuco";

    /**
     * @brief Detects interpolated ChArUco corners and their board coordinates.
     * @param image Camera image.
     * @param board ChArUco configuration.
     * @return Observation with corresponding image and object points, or
     * `std::nullopt` if fewer than four ChArUco corners are found.
     */
    std::optional<data::observation> detect(
        const cv::Mat& image, const data::board::config& board) const override;
};

}  // namespace clover2_camera_utils::calibration::detector
