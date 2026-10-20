#pragma once

#include <clover2_camera_utils/calibration/calibration_algorithm.hpp>

namespace clover2_camera_utils::calibration {

class fisheye_calibration_algorithm final : public calibration_algorithm {
public:
    std::optional<data::calibration_result> calibrate(
        const std::vector<data::observation>& observations) const override;
};

}  // namespace clover2_camera_utils::calibration