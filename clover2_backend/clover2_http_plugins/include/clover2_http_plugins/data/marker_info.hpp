#pragma once

// clover2
#include <clover2_http_plugins/data/marker_pose.hpp>
#include <clover2_http_plugins/utils/json_adl.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <optional>
#include <string>

namespace clover2_http_plugins::data {

struct marker_info {
    int id = 0;
    std::string type = "fixed";
    double size = -1.0;
    std::string marker_frame_id;
    std::optional<marker_pose> pose;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(marker_info, id, type, size,
                                                marker_frame_id, pose);

}  // namespace clover2_http_plugins::data
