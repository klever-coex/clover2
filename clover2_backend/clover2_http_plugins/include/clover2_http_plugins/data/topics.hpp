#pragma once

#include <clover2_http_plugins/data/topic_info.hpp>

#include <nlohmann/json.hpp>

#include <string>

namespace clover2_http_plugins::data {

struct topics {
    std::vector<topic_info> topics;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(topics, topics)

}  // namespace clover2_http_plugins::data
