#pragma once

// ROS2
#include <lifecycle_msgs/msg/state.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <cstdint>
#include <string>

namespace clover2_http::plugins::data {

// class not struct for correctly serializing/deserializing to JSON as string
class lifecycle_state {
public:
    lifecycle_state() = default;
    lifecycle_state(uint8_t id, std::string label);
    explicit lifecycle_state(const lifecycle_msgs::msg::State& msg);

    lifecycle_state& operator=(const lifecycle_msgs::msg::State& msg);

    uint8_t id() const;
    const std::string& label() const;

private:
    uint8_t m_id = lifecycle_msgs::msg::State::PRIMARY_STATE_UNKNOWN;
    std::string m_label;
};

void to_json(nlohmann::json& j, const lifecycle_state& state);

void from_json(const nlohmann::json& j, lifecycle_state& state);

}  // namespace clover2_http::plugins::data
