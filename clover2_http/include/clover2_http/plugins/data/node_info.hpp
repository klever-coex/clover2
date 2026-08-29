#pragma once

// clover2
#include <clover2_http/plugins/utils/json_adl.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <optional>
#include <string>

namespace clover2_http::plugins::data {

inline std::string full_node_name(const std::string& ns,
                                  const std::string& name) {
    return (ns == "/") ? ns + name : ns + "/" + name;
}

struct node_info {
    std::string name;
    std::string ns;
    bool is_lifecycle = false;
    std::optional<std::string> lifecycle_state = std::nullopt;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(node_info, name, ns, is_lifecycle, lifecycle_state)

}  // namespace clover2_http::plugins::data
