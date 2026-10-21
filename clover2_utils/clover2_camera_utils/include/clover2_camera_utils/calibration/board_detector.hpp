#pragma once

#include <clover2_camera_utils/data/board_config.hpp>
#include <clover2_camera_utils/data/observation.hpp>

#include <opencv2/core/mat.hpp>

#include <optional>

namespace clover2_camera_utils::calibration {

class board_detector {
public:
    virtual ~board_detector() = default;

    virtual std::optional<data::observation> detect(
        const cv::Mat& image, const data::board_config& board) const = 0;
};

}  // namespace clover2_camera_utils::calibration
