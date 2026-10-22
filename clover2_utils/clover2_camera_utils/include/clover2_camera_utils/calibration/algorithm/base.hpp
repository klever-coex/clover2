#pragma once

#include <clover2_camera_utils/data/calibration_result.hpp>
#include <clover2_camera_utils/data/observation.hpp>

#include <optional>
#include <vector>

namespace clover2_camera_utils::calibration::algorithm {

class base {
public:
    virtual ~base() = default;

    virtual std::optional<data::calibration_result> calibrate(
        const std::vector<data::observation>& observations) const = 0;
};

}  // namespace clover2_camera_utils::calibration::algorithm
