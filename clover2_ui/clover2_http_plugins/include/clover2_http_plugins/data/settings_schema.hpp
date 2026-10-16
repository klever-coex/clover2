#pragma once

// JSON
#include <nlohmann/json.hpp>

namespace clover2_http_plugins::data {

struct settings_schema {
    bool valid = false;
    nlohmann::json root;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(settings_schema, valid, root)

}  // namespace clover2_http_plugins::data
