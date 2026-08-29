#pragma once

#include <nlohmann/json.hpp>

#include <string>

namespace clover2_http::plugins::data {

struct parameter {
    std::string name;
    std::string description;
    
    bool read_only;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(parameter, name, description, read_only)

}  // namespace clover2_http::plugins::data
