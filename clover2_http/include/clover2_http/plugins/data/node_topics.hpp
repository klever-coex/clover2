#pragma once

#include <clover2_http/plugins/data/topic_endpoint.hpp>

#include <nlohmann/json.hpp>

namespace clover2_http::plugins::data {

struct node_topics {
    std::vector<topic_endpoint> topics;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(node_topics, topics)

}  // namespace clover2_http::plugins::data
