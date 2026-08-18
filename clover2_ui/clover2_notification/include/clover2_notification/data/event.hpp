#pragma once

#include <diagnostic_msgs/msg/diagnostic_status.hpp>

#include <cstdint>
#include <string>

namespace clover2_notification::data {

struct event {
    event() = default;

    explicit event(const diagnostic_msgs::msg::DiagnosticStatus& status)
        : name(status.name)
        , level(status.level)
        , message(status.message) {}

    std::string name;
    uint8_t level{};
    std::string message;

    bool operator==(const event& other) const {
        return name == other.name && level == other.level &&
               message == other.message;
    }

    bool operator!=(const event& other) const { return !(*this == other); }
};

}  // namespace clover2_notification::data