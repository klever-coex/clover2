#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace clover2_http_plugins::data {

struct service_info {
    std::string name;
    std::string type;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(service_info, name, type)

}  // namespace clover2_http_plugins::data
