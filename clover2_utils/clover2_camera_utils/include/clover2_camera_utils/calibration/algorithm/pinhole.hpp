#pragma once

#include <clover2_camera_utils/calibration/algorithm/base.hpp>

namespace clover2_camera_utils::calibration::algorithm {

class pinhole final : public base {
public:
    std::optional<data::calibration_result> calibrate(
        const std::vector<data::observation>& observations) const override;
};

}  // namespace clover2_camera_utils::calibration::algorithm
