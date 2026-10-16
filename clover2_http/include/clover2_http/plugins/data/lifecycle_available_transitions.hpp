#pragma once

// clover2
#include <clover2_http/plugins/data/lifecycle_transition_description.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <vector>

namespace clover2_http::plugins::data {

struct lifecycle_available_transitions {
    std::vector<lifecycle_transition_description> available_transitions;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(lifecycle_available_transitions,
                                   available_transitions)

}  // namespace clover2_http::plugins::data
