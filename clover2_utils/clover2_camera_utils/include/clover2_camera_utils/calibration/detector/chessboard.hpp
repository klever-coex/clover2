#pragma once

#include <clover2_camera_utils/calibration/detector/base.hpp>

#include <opencv2/core/mat.hpp>

#include <optional>

namespace clover2_camera_utils::calibration::detector {

class chessboard final : public base {
public:
    std::optional<data::observation> detect(
        const cv::Mat& image, const data::board_config& board) const override;
};

}  // namespace clover2_camera_utils::calibration::detector
