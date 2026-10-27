/**
 * @file chessboard.hpp
 * @brief Provides a detector for chessboard boards.
 */

#pragma once

#include <clover2_camera_utils/calibration/detector/base.hpp>

#include <opencv2/core/mat.hpp>

#include <optional>

namespace clover2_camera_utils::calibration::detector {

/** @brief Detects inner corners of a chessboard. */
class chessboard final : public base {
public:
    static constexpr const char* name = "chessboard";

    /**
     * @brief Detects a chessboard in an image.
     * @param image Camera image.
     * @param board Chessboard configuration.
     * @return Observation with refined corners or `std::nullopt` if the
     * chessboard is not detected or parameters are invalid.
     */
    std::optional<data::observation> detect(
        const cv::Mat& image, const data::board_config& board) const override;
};

}  // namespace clover2_camera_utils::calibration::detector
