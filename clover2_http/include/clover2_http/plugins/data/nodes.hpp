#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace clover2_http::plugins::data {

struct nodes {
    std::vector<std::string> nodes;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(nodes, nodes)

}  // namespace clover2_http::plugins::data
