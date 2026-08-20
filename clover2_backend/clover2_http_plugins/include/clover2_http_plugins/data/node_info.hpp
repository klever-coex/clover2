#pragma once

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
    bool is_lifecycle = false;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(node_info, name, ns, is_lifecycle)

}  // namespace clover2_http_plugins::data
