/**
 * @file output.hpp
 * @brief Provides the base notification output interface and event queue
 * handling.
 */

#pragma once

// clover2
#include <clover2_notification/data/event.hpp>

// ROS2
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// STL
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <vector>

namespace clover2_notification {

/**
 * @class output
 * @brief Base class for notification output plugins.
 *
 * The class owns a priority queue of notification events and guarantees that
 * only one event is processed at a time. Derived classes implement the actual
 * delivery mechanism in process_event() and call the provided completion
 * callback when the event has finished processing.
 */
class output {
public:
    /** @brief Callback that marks the current event as processed. */
    using done_callback = std::function<void()>;

    /** @brief Destroy the output plugin. */
    virtual ~output() = default;

    /**
     * @brief Initialize the output plugin.
     *
     * @param node Lifecycle node that owns the plugin.
     * @param id Output instance identifier used as a parameter namespace.
     */
    virtual void initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
        const std::string& id) = 0;

    /**
     * @brief Add an event to the output queue and start processing if idle.
     *
     * Events with a higher priority value are processed before events with
     * lower priority values.
     *
     * @param event Notification event to enqueue.
     */
    void push2queue(const data::event& event) {
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_queue.push(event);
        }

        try_process_next();
    }

    /** @brief Clear queued and currently tracked events. */
    virtual void clear() {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_queue = queue_type{};
        m_current_event.reset();
    }

protected:
    /**
     * @brief Process a single notification event.
     *
     * Implementations must call @p done after the event has been fully
     * processed so the base class can continue with the next queued event.
     *
     * @param event Event to process.
     * @param done Completion callback.
     */
    virtual void process_event(const data::event& event,
                               done_callback done) = 0;

    /**
     * @brief Get the number of events waiting in the queue.
     *
     * @return Number of queued events, excluding the currently processed event.
     */
    size_t queued_size() const {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        return m_queue.size();
    }

private:
    /** @brief Priority comparator that puts higher-priority events first. */
    struct priority_less {
        /**
         * @brief Compare two events by priority.
         *
         * @param lhs Left-hand event.
         * @param rhs Right-hand event.
         * @return true if @p lhs has lower priority than @p rhs.
         */
        bool operator()(const data::event& lhs, const data::event& rhs) const {
            return lhs.priority < rhs.priority;
        }
    };

    using queue_type =
        std::priority_queue<data::event, std::vector<data::event>,
                            priority_less>;

    /** @brief Start processing the next queued event when no event is active.
     */
    void try_process_next() {
        data::event event;
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            if (m_current_event || m_queue.empty()) {
                return;
            }

            m_current_event = m_queue.top();
            event = *m_current_event;
            m_queue.pop();
        }

        process_event(event, [this]() {
            complete_current_event();
        });
    }

    /** @brief Mark the current event as completed and process the next one. */
    void complete_current_event() {
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_current_event.reset();
        }

        try_process_next();
    }

    queue_type m_queue;
    std::optional<data::event> m_current_event;
    mutable std::mutex m_queue_mutex;
};

}  // namespace clover2_notification
