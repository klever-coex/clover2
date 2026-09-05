/**
 * @file output.hpp
 * @brief Provides the base notification output interface and event queue
 * handling.
 */

#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_notification/data/event.hpp>

// ROS2
#include <rclcpp/create_timer.hpp>

// STL
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
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
     * @param node_context Shared node context used to access ROS 2 interfaces.
     * @param id Output instance identifier used as a parameter namespace.
     *
     * @throws std::invalid_argument if @p node_context is null or @p id is
     * empty.
     */
    void initialize(std::shared_ptr<clover2_common::node_context> node_context,
                    const std::string& id) {
        if (!node_context) {
            throw std::invalid_argument(
                "Notification output received a null node context");
        }

        if (id.empty()) {
            throw std::invalid_argument(
                "Notification output received an empty id");
        }

        m_id = id;
        m_node_context = std::move(node_context);
        on_initialize();
    }

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
        m_queue = std::priority_queue<data::event>{};
        m_current_event.reset();
    }

protected:
    /**
     * @brief Get the output instance identifier.
     *
     * @return Output instance identifier used as a parameter namespace.
     */
    const std::string& id() const { return m_id; }

    /**
     * @brief Get shared node context used by the output plugin.
     *
     * @return Shared node context used to access ROS 2 interfaces.
     */
    const std::shared_ptr<clover2_common::node_context>& node_context() const {
        return m_node_context;
    }

    /**
     * @brief Declare and read a parameter through node context.
     *
     * @param name Fully-qualified parameter name.
     * @param default_value Parameter default value.
     * @return Declared parameter value.
     */
    template <typename T>
    T declare_parameter(const std::string& name, const T& default_value) const {
        return rclcpp::node_interfaces::get_node_parameters_interface(
                   m_node_context)
            ->declare_parameter(name, rclcpp::ParameterValue(default_value))
            .get<T>();
    }

    /**
     * @brief Declare and read an output-local parameter.
     *
     * @param name Parameter name relative to the output id namespace.
     * @param default_value Parameter default value.
     * @return Declared parameter value.
     */
    template <typename T>
    T declare_output_parameter(const std::string& name,
                               const T& default_value) const {
        return declare_parameter<T>(id() + "." + name, default_value);
    }

    /**
     * @brief Create a wall timer using the output node context.
     *
     * @param period Timer period.
     * @param callback Timer callback.
     * @return Created timer.
     */
    template <typename DurationT, typename CallbackT>
    rclcpp::TimerBase::SharedPtr create_timer(DurationT period,
                                              CallbackT&& callback) const {
        return rclcpp::create_timer(
            m_node_context->get_node_base_interface(),
            m_node_context->get_node_timers_interface(),
            m_node_context->get_node_clock_interface()->get_clock(), period,
            std::forward<CallbackT>(callback));
    }

    /**
     * @brief Perform implementation-specific initialization.
     *
     * This method is called by initialize() after common validation and id
     * storage are complete.
     *
     */
    virtual void on_initialize() = 0;

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

        process_event(event, [this]() { complete_current_event(); });
    }

    /** @brief Mark the current event as completed and process the next one. */
    void complete_current_event() {
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_current_event.reset();
        }

        try_process_next();
    }

    std::string m_id;
    std::shared_ptr<clover2_common::node_context> m_node_context;
    std::priority_queue<data::event> m_queue;
    std::optional<data::event> m_current_event;
    mutable std::mutex m_queue_mutex;
};

}  // namespace clover2_notification
