#pragma once

// JSON
#include <nlohmann/json.hpp>

// STL
#include <string>

namespace clover2_http_plugins::data {

struct modify_result {
    bool success = false;
    std::string error_message;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(modify_result, success, error_message)

}  // namespace clover2_http_plugins::data
