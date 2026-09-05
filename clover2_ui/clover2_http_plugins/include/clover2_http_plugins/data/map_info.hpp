#pragma once

// clover2
#include <clover2_http_plugins/data/marker_info.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <string>
#include <vector>

namespace clover2_http_plugins::data {

struct map_info {
    bool valid = false;
    std::string name;
    std::string frame_id;
    std::string dictionary;
    int count = 0;
    std::vector<marker_info> markers;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(map_info, valid, name, frame_id,
                                   dictionary, count, markers)

}  // namespace clover2_http_plugins::data
