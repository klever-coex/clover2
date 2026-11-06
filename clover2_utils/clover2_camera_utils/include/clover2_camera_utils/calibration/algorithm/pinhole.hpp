/**
 * @file pinhole.hpp
 * @brief Provides a calibration algorithm for the pinhole model.
 */

#pragma once

#include <clover2_camera_utils/calibration/algorithm/base.hpp>

namespace clover2_camera_utils::calibration::algorithm {

/** @brief Performs camera calibration for the pinhole model. */
class pinhole final : public base {
public:
    static constexpr const char* name = "pinhole";

    /**
     * @brief Performs calibration using observations.
     * @param observations Calibration board observations.
     * @return Camera information and RMS reprojection error in pixels, or
     * `std::nullopt` if observations are invalid or calculation fails.
     */
    std::optional<result> calibrate(
        const std::vector<data::observation>& observations) const override;
};

}  // namespace clover2_camera_utils::calibration::algorithm
