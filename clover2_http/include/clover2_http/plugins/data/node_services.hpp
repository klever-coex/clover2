#pragma once

#include <clover2_http/plugins/data/service_endpoint.hpp>

#include <nlohmann/json.hpp>

#include <vector>

namespace clover2_http::plugins::data {

struct node_services {
    std::vector<service_endpoint> services;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(node_services, services)

}  // namespace clover2_http::plugins::data
