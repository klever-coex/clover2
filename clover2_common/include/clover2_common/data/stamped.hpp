#pragma once

// ROS2
#include <rclcpp/time.hpp>

// STL
#include <optional>

namespace clover2_common::data {

template <typename T>
struct stamped {
    std::optional<T> value;
    rclcpp::Time stamp{static_cast<int64_t>(0), RCL_ROS_TIME};
};

}  // namespace clover2_common::data
