#pragma once

// clover2
#include <clover2_fcu_bridge/data/stamped.hpp>

// ROS2
#include <sensor_msgs/msg/imu.hpp>

namespace clover2_fcu_bridge::data {

using imu_data = stamped<sensor_msgs::msg::Imu>;

}  // namespace clover2_fcu_bridge::data