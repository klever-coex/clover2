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
    static constexpr const char* name = "fisheye";

    /**
     * @brief Performs calibration using observations.
     * @param observations Calibration board observations.
     * @return Camera information or `std::nullopt` if observations are invalid
     * or calculation fails.
     */
    std::optional<sensor_msgs::msg::CameraInfo> calibrate(
        const std::vector<data::observation>& observations) const override;
};

}  // namespace clover2_camera_utils::calibration::algorithm
