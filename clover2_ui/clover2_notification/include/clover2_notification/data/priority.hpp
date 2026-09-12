/**
 * @file priority.hpp
 * @brief Provides standard notification event priority levels.
 */

#pragma once

namespace clover2_notification::data {

/** @brief Standard priority levels for notification status events. */
enum class priority : int {
    /** @brief Normal status without an alert. */
    ok = 0,

    /** @brief Warning status. */
    warning = 1,

    /** @brief Error status. */
    error = 2,

    /** @brief Stale status. */
    stale = 3,
};

}  // namespace clover2_notification::data
