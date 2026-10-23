/**
 * @file fisheye.hpp
 * @brief Provides a calibration algorithm for the fisheye model.
 */

#pragma once

#include <clover2_camera_utils/calibration/algorithm/base.hpp>

namespace clover2_camera_utils::calibration::algorithm {

/** @brief Performs camera calibration for the fisheye model. */
class fisheye final : public base {
public:
    /**
     * @brief Performs calibration using observations.
     * @param observations Calibration board observations.
     * @return Calibration result or `std::nullopt` if observations are invalid
     * or calculation fails.
     */
    std::optional<data::calibration_result> calibrate(
        const std::vector<data::observation>& observations) const override;
};

}  // namespace clover2_camera_utils::calibration::algorithm
