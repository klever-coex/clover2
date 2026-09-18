#pragma once

// JSON
#include <nlohmann/json.hpp>

namespace clover2_http_plugins::data {

struct marker_pose {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    double roll = 0.0;
    double pitch = 0.0;
    double yaw = 0.0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(marker_pose, x, y, z, roll,
                                                pitch, yaw)

}  // namespace clover2_http_plugins::data
