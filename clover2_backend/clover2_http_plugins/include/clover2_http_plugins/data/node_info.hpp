#pragma once

#include <clover2_http_plugins/data/topic_info.hpp>

#include <nlohmann/json.hpp>

#include <string>

namespace clover2_http_plugins::data {

inline std::string full_node_name(const std::string& ns,
                                  const std::string& name) {
    return (ns == "/") ? ns + name : ns + "/" + name;
}

struct node_info {
    std::string name;
    std::string ns;
    std::vector<topic_info> publishers;
    std::vector<topic_info> subscribers;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(node_info, name, ns, publishers, subscribers)

}  // namespace clover2_http_plugins::data
