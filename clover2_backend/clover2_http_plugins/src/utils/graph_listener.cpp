// clover2
#include <clover2_http_plugins/utils/graph_listener.hpp>

// ROS2
#include <rclcpp/logging.hpp>
#include <rclcpp/utilities.hpp>

// STL
#include <chrono>
#include <exception>
#include <pthread.h>
#include <utility>

using namespace std::chrono_literals;

namespace clover2_http_plugins::utils {

graph_listener::graph_listener(
    std::shared_ptr<clover2_common::node_context> node_context, callback&& cb)
    : m_node_context(std::move(node_context))
    , m_callback(std::move(cb)) {
    m_event = m_node_context->get_node_graph_interface()->get_graph_event();
    m_thread = std::thread(&graph_listener::run, this);
}

graph_listener::~graph_listener() {
    m_stop = true;

    if (m_thread.joinable()) {
        m_thread.join();
    }
}

void graph_listener::run() {
    auto graph = m_node_context->get_node_graph_interface();

    while (rclcpp::ok() && !m_stop) {
        graph->wait_for_graph_change(m_event, 100ms);
        if (!m_event->check_and_clear() || !m_callback) {
            continue;
        }

        try {
            m_callback();
        } catch (const std::exception& e) {
            RCLCPP_ERROR(m_node_context->get_logger(),
                         "graph_listener callback failed: %s", e.what());
        } catch (...) {
            RCLCPP_ERROR(
                m_node_context->get_logger(),
                "graph_listener callback failed with an unknown error");
        }
    }
}

}  // namespace clover2_http_plugins::utils
