#pragma once

// clover2
#include <clover2_http/plugins/data/topic_info.hpp>

// JSON
#include <nlohmann/json.hpp>

namespace clover2_http::plugins::data {

struct topics {
    std::vector<topic_info> topics;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(topics, topics)

}  // namespace clover2_http::plugins::data
