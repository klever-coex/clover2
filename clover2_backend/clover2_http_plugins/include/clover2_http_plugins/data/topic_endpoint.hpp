#pragma once

#include <clover2_http_plugins/data/qos.hpp>
#include <clover2_http_plugins/data/topic_info.hpp>

#include <nlohmann/json.hpp>

namespace clover2_http_plugins::data {

struct topic_endpoint {
    topic_info info;
    qos qos_profile;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(topic_endpoint, info, qos_profile)

}  // namespace clover2_http_plugins::data
