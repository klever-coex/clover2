// clover2
#include <clover2_http/plugins/data/lifecycle_state.hpp>

// STL
#include <cstdint>
#include <unordered_map>
#include <utility>

namespace clover2_http::plugins::data {

namespace {

static const std::unordered_map<std::string, uint8_t> ids = {
    {"unconfigured", lifecycle_msgs::msg::State::PRIMARY_STATE_UNCONFIGURED},
    {"inactive", lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE},
    {"active", lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE},
    {"finalized", lifecycle_msgs::msg::State::PRIMARY_STATE_FINALIZED},
};

}  // namespace

lifecycle_state::lifecycle_state(uint8_t id, std::string label)
    : m_id(id)
    , m_label(std::move(label)) {}

lifecycle_state::lifecycle_state(const lifecycle_msgs::msg::State& msg)
    : m_id(msg.id)
    , m_label(msg.label) {}

lifecycle_state& lifecycle_state::operator=(
    const lifecycle_msgs::msg::State& msg) {
    m_id = msg.id;
    m_label = msg.label;

    return *this;
}

uint8_t lifecycle_state::id() const { return m_id; }

const std::string& lifecycle_state::label() const { return m_label; }

void to_json(nlohmann::json& j, const lifecycle_state& state) {
    j["label"] = state.label();
}

void from_json(const nlohmann::json& j, lifecycle_state& state) {
    const std::string label = j["label"].get<std::string>();

    const auto found = ids.find(label);
    const uint8_t id = (found != ids.cend())
                           ? found->second
                           : lifecycle_msgs::msg::State::PRIMARY_STATE_UNKNOWN;

    state = lifecycle_state(id, label);
}

}  // namespace clover2_http::plugins::data
