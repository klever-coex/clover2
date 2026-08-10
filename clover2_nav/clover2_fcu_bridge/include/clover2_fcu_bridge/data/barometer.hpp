#pragma once

// clover2
#include <clover2_fcu_bridge/data/stamped.hpp>

// ROS2
#include <sensor_msgs/msg/fluid_pressure.hpp>

namespace clover2_fcu_bridge::data {

using barometer_data = stamped<sensor_msgs::msg::FluidPressure>;

}  // namespace clover2_fcu_bridge::data