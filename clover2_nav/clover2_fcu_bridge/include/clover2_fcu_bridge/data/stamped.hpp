#pragma once

// ROS2
#include <rclcpp/time.hpp>

// STL
#include <optional>

namespace clover2_fcu_bridge::data {

template <typename T>
struct stamped {
    std::optional<T> value;
    rclcpp::Time stamp;
};

}  // namespace clover2_fcu_bridge::data