#pragma once

// clover2
#include <clover2_http/plugins/data/lifecycle_state.hpp>
#include <clover2_http/plugins/data/lifecycle_transition.hpp>

// ROS2
#include <lifecycle_msgs/msg/transition_description.hpp>

// JSON
#include <nlohmann/json.hpp>

namespace clover2_http::plugins::data {

struct lifecycle_transition_description {
    lifecycle_transition transition;
    lifecycle_state start_state;
    lifecycle_state goal_state;

    lifecycle_transition_description() = default;

    explicit lifecycle_transition_description(
        const lifecycle_msgs::msg::TransitionDescription& msg);
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(lifecycle_transition_description,
                                   transition, start_state, goal_state)

}  // namespace clover2_http::plugins::data
