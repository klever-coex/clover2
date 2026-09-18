#pragma once

// JSON
#include <nlohmann/json.hpp>

namespace clover2_http_plugins::data {

struct settings_values {
    nlohmann::json values;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(settings_values, values)

}  // namespace clover2_http_plugins::data
