/**
 * @file event.hpp
 * @brief Provides notification event data structure.
 */

#pragma once

// STL
#include <string>

namespace clover2_notification::data {

/**
 * @struct event
 * @brief Notification event emitted by providers and consumed by outputs.
 */
struct event {
    /** @brief Event priority. Higher values are processed first by outputs. */
    int priority{};

    /** @brief Event source identifier, for example a provider name. */
    std::string source;

    /** @brief Event name, for example a diagnostic status name. */
    std::string name;

    /** @brief Human-readable event message. */
    std::string message;

    /**
     * @brief Compare two notification events for equality.
     *
     * @param other Event to compare with.
     * @return true if all event fields are equal, false otherwise.
     */
    bool operator==(const event& other) const {
        return priority == other.priority && source == other.source &&
               name == other.name && message == other.message;
    }

    /**
     * @brief Compare two notification events for inequality.
     *
     * @param other Event to compare with.
     * @return true if at least one event field differs, false otherwise.
     */
    bool operator!=(const event& other) const {
        return !(*this == other);
    }
};

}  // namespace clover2_notification::data
