#pragma once

// ROS2
#include <lifecycle_msgs/msg/transition.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL
#include <cstdint>
#include <string>

namespace clover2_http::plugins::data {

/*
Serializes as JSON object with "label" field, e.g.:

{
  "label": "cleanup"
}
*/

struct lifecycle_transition {
    lifecycle_transition() = default;
    lifecycle_transition(uint8_t id, std::string label);
    explicit lifecycle_transition(const lifecycle_msgs::msg::Transition& msg);

    lifecycle_transition& operator=(const lifecycle_msgs::msg::Transition& msg);

    uint8_t id() const;
    const std::string& label() const;

private:
    uint8_t m_id = lifecycle_msgs::msg::Transition::TRANSITION_CREATE;
    std::string m_label;
};

void to_json(nlohmann::json& j, const lifecycle_transition& transition);

void from_json(const nlohmann::json& j, lifecycle_transition& transition);

}  // namespace clover2_http::plugins::data
