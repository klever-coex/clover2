#pragma once

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <cstdint>
#include <optional>

namespace clover2_fcu::data {

struct optical_flow {
    uint8_t quality;
    /// Sampling period of the flow sensor [us].
    uint32_t integration_time_us;

    /// Integrated optical flow since the previous sample [rad].
    std::optional<double> integrated_x;
    std::optional<double> integrated_y;

    /// Angular rates measured during the integration window [rad/s].
    std::optional<double> integrated_xgyro;
    std::optional<double> integrated_ygyro;
    std::optional<double> integrated_zgyro;

    /// Distance to the surface [m], when the sensor provides it.
    std::optional<double> distance;

    rclcpp::Time stamp{};
};

}  // namespace clover2_fcu::data
