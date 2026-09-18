#pragma once

// clover2
#include <clover2_http/data/plugin_info.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <string>
#include <vector>

namespace clover2_http::data {

struct manifest {
    std::string framework_version;
    std::vector<plugin_info> plugins;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(manifest, framework_version, plugins)

}  // namespace clover2_http::data
