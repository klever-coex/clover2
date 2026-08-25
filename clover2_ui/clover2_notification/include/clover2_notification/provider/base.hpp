/**
 * @file base.hpp
 * @brief Provides the base notification provider interface.
 */

#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_notification/data/event.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <functional>
#include <memory>

namespace clover2_notification::provider {

/** @typedef callback_type
 *  @brief Callback used by providers to emit notification events.
 */
using callback_type = std::function<void(const data::event&)>;

/**
 * @interface base
 * @brief Base interface for notification event providers.
 */
class base {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(base)

    /** @brief Destroy the provider. */
    virtual ~base() = default;

    /**
     * @brief Initialize the provider.
     *
     * @param node_context Shared node context used to access ROS 2 node
     * interfaces.
     * @param callback Callback invoked when the provider emits an event.
     */
    virtual void initialize(
        std::shared_ptr<clover2_common::node_context> node_context,
        callback_type callback) = 0;

    /** @brief Stop the provider and release runtime resources. */
    virtual void cleanup() = 0;
};

}  // namespace clover2_notification::provider
