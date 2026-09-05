// clover2
#include <clover2_http/plugins/data/lifecycle_transition_description.hpp>

namespace clover2_http::plugins::data {

lifecycle_transition_description::lifecycle_transition_description(
    const lifecycle_msgs::msg::TransitionDescription& msg)
    : transition(msg.transition)
    , start_state(msg.start_state)
    , goal_state(msg.goal_state) {}

}  // namespace clover2_http::plugins::data
