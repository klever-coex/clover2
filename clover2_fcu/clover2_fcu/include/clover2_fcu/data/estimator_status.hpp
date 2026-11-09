#pragma once

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_fcu::data {

/// FCU estimator health snapshot (no mavros source in the MVP — not fed).
struct estimator_status {
    bool healthy{false};
    bool local_valid{false};
    bool global_valid{false};

    /// External inputs currently fused by the estimator.
    bool vision_fused{false};
    bool gps_fused{false};

    /// Normalized innovation test ratio (> 1 means the measurement
    /// disagrees with the estimate).
    float innovation_ratio{0.f};
    rclcpp::Time stamp{};
};

}  // namespace clover2_fcu::data
