#pragma once

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_fcu::data {

struct estimator_status {
    bool healthy{false};
    bool local_valid{false};
    bool global_valid{false};
    bool vision_fused{false};
    bool gps_fused{false};
    float innovation_ratio{0.f};
    rclcpp::Time stamp{};
};

}  // namespace clover2_fcu::data
