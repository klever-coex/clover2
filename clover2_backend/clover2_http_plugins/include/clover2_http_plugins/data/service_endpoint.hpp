#pragma once

// clover2
#include <clover2_http_plugins/data/service_info.hpp>

// JSON
#include <nlohmann/json.hpp>

namespace clover2_http_plugins::data {

struct service_endpoint {
    service_info info;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(service_endpoint, info)

}  // namespace clover2_http_plugins::data
