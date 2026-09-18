#pragma once

// clover2
#include <clover2_http/plugins/data/lifecycle_state.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <optional>
#include <string>

namespace clover2_http::plugins::data {

std::string full_node_name(const std::string& ns, const std::string& name);

struct node_info {
    std::string name;
    std::string ns;
    bool is_lifecycle = false;
    std::optional<class lifecycle_state> lifecycle_state = std::nullopt;
};

void to_json(nlohmann::json& json, const node_info& info);

void from_json(const nlohmann::json& json, node_info& info);

}  // namespace clover2_http::plugins::data
