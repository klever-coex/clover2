#pragma once

#include <string>

namespace clover2_notification::data {

struct event {
    int priority{};
    std::string source;
    std::string name;
    std::string message;

    bool operator==(const event& other) const {
        return priority == other.priority && source == other.source &&
               name == other.name && message == other.message;
    }

    bool operator!=(const event& other) const {
        return !(*this == other);
    }
};

}  // namespace clover2_notification::data
