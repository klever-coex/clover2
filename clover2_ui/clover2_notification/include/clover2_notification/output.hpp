#pragma once

#include <clover2_notification/data/event.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <vector>

namespace clover2_notification {

class output {
public:
    using done_callback = std::function<void()>;

    virtual ~output() = default;

    virtual void initialize(const rclcpp_lifecycle::LifecycleNode::SharedPtr& node,
                            const std::string& id) = 0;
    void push2queue(const data::event& event) {
        {
            std::lock_guard<std::mutex> lock(m_queue_mutex);
            m_queue.push(event);
        }

        try_process_next();
    }

    virtual void clear() {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_queue = queue_type{};
        m_current_event.reset();
    }

protected:
    virtual void process_event(const data::event& event,
                               done_callback done) = 0;

    size_t queued_size() const {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        return m_queue.size();
    }

private:
    struct priority_less {
        bool operator()(const data::event& lhs, const data::event& rhs) const {
            return lhs.priority < rhs.priority;
        }
    };

    using queue_type =
        std::priority_queue<data::event, std::vector<data::event>,
                            priority_less>;

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
