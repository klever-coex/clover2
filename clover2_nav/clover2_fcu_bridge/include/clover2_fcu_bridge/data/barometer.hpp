#pragma once

// clover2
#include <clover2_common/data/stamped.hpp>

// ROS2
#include <sensor_msgs/msg/fluid_pressure.hpp>

namespace clover2_fcu_bridge::data {

using barometer_data =
    clover2_common::data::stamped<sensor_msgs::msg::FluidPressure>;

}  // namespace clover2_fcu_bridge::data
