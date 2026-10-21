#pragma once

#include <clover2_camera_utils/calibration/board_detector.hpp>

#include <opencv2/core/mat.hpp>

#include <optional>

namespace clover2_camera_utils::calibration {

class chessboard_detector final : public board_detector {
public:
    std::optional<data::observation> detect(
        const cv::Mat& image, const data::board_config& board) const override;
};

}  // namespace clover2_camera_utils::calibration
