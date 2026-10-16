#pragma once

// JSON
#include <nlohmann/json.hpp>

// STL
#include <string>

namespace clover2_http::plugins::data {

struct topic_info {
    std::string name;
    std::string type;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(topic_info, name, type)

}  // namespace clover2_http::plugins::data
