// clover2
#include <clover2_http/plugins/data/lifecycle_transition.hpp>

// STL
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace clover2_http::plugins::data {

namespace {

static const std::unordered_map<std::string, uint8_t> ids = {
    {"create", lifecycle_msgs::msg::Transition::TRANSITION_CREATE},
    {"configure", lifecycle_msgs::msg::Transition::TRANSITION_CONFIGURE},
    {"cleanup", lifecycle_msgs::msg::Transition::TRANSITION_CLEANUP},
    {"activate", lifecycle_msgs::msg::Transition::TRANSITION_ACTIVATE},
    {"deactivate", lifecycle_msgs::msg::Transition::TRANSITION_DEACTIVATE},
    {"unconfigured_shutdown",
     lifecycle_msgs::msg::Transition::TRANSITION_UNCONFIGURED_SHUTDOWN},
    {"inactive_shutdown",
     lifecycle_msgs::msg::Transition::TRANSITION_INACTIVE_SHUTDOWN},
    {"active_shutdown",
     lifecycle_msgs::msg::Transition::TRANSITION_ACTIVE_SHUTDOWN},
    {"shutdown", lifecycle_msgs::msg::Transition::TRANSITION_CREATE},
    {"destroy", lifecycle_msgs::msg::Transition::TRANSITION_DESTROY},
};

}  // namespace

lifecycle_transition::lifecycle_transition(uint8_t id, std::string label)
    : m_id(id)
    , m_label(std::move(label)) {}

lifecycle_transition::lifecycle_transition(
    const lifecycle_msgs::msg::Transition& msg)
    : m_id(msg.id)
    , m_label(msg.label) {}

lifecycle_transition& lifecycle_transition::operator=(
    const lifecycle_msgs::msg::Transition& msg) {
    m_id = msg.id;
    m_label = msg.label;

    return *this;
}

uint8_t lifecycle_transition::id() const { return m_id; }

const std::string& lifecycle_transition::label() const { return m_label; }

void to_json(nlohmann::json& j, const lifecycle_transition& transition) {
    j["label"] = transition.label();
}

void from_json(const nlohmann::json& j, lifecycle_transition& transition) {
    const std::string label = j["label"].get<std::string>();

    const auto found = ids.find(label);
    if (found == ids.cend()) {
        throw std::invalid_argument("Unknown lifecycle transition: '" + label +
                                    "'");
    }

    transition = lifecycle_transition(found->second, label);
}

}  // namespace clover2_http::plugins::data
