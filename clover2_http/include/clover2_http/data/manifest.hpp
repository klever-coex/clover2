#pragma once

#include <clover2_http/data/plugin_info.hpp>

#include <nlohmann/json.hpp>

#include <vector>

namespace clover2_http::data {

struct manifest {
    std::vector<plugin_info> plugins;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(manifest, plugins)

}  // namespace clover2_http::data
