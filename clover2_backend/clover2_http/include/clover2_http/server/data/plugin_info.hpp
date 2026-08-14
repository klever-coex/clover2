#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace clover2_http::server::data {

struct plugin_info {
    std::string name;
    int version = 0;
    std::vector<std::string> capabilities;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(plugin_info, name, version, capabilities)

}  // namespace clover2_http::server::data
