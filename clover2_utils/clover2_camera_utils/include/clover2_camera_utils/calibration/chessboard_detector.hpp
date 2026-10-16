#pragma once

#include <clover2_camera_utils/data/board_config.hpp>
#include <clover2_camera_utils/data/observation.hpp>

#include <opencv2/core/mat.hpp>

#include <optional>

namespace clover2_camera_utils::calibration {

class chessboard_detector {
public:
    std::optional<data::observation> detect(
        const cv::Mat& image, const data::board_config& board) const;
};

}  // namespace clover2_camera_utils::calibration
