/**
 * @file base.hpp
 * @brief Provides the base algorithm interface for camera calibration.
 */

#pragma once

#include <clover2_camera_utils/data/calibration_result.hpp>
#include <clover2_camera_utils/data/observation.hpp>

#include <optional>
#include <vector>

namespace clover2_camera_utils::calibration::algorithm {

/** @brief Defines the algorithm interface for camera calibration. */
class base {
public:
    virtual ~base() = default;

    /**
     * @brief Performs calibration using observations.
     * @param observations Calibration board observations.
     * @return Calibration result or `std::nullopt` if calculation is not
     * possible.
     */
    virtual std::optional<data::calibration_result> calibrate(
        const std::vector<data::observation>& observations) const = 0;
};

}  // namespace clover2_camera_utils::calibration::algorithm
