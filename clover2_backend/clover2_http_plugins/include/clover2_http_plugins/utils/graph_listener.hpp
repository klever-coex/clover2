#pragma once

// clover2
#include <clover2_common/node_context.hpp>

// ROS2
#include <rclcpp/event.hpp>
#include <rclcpp/timer.hpp>

// STL
#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace clover2_http_plugins::utils {

class graph_listener {
public:
    using callback = std::function<void()>;

    graph_listener(std::shared_ptr<clover2_common::node_context> node_context,
                   callback&& cb);
    ~graph_listener();

    graph_listener(const graph_listener&) = delete;
    graph_listener& operator=(const graph_listener&) = delete;

private:
    void run();

    std::shared_ptr<clover2_common::node_context> m_node_context;

    callback m_callback;

    rclcpp::TimerBase::SharedPtr m_timer;
    
    std::thread m_thread;
    rclcpp::Event::SharedPtr m_event;
    std::atomic<bool> m_stop{false};
};

}  // namespace clover2_http_plugins::utils
