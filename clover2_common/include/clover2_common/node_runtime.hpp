/**
 * @file node_runtime.hpp
 * @brief Provides project node runtime.
 */

#pragma once

#include <clover2_common/node.hpp>
#include <clover2_common/node_context.hpp>
#include <rclcpp/executors/single_threaded_executor.hpp>
#include <rclcpp/macros.hpp>
#include <rclcpp/node_options.hpp>

#include <memory>
#include <string>
#include <thread>

namespace clover2_common {

/**
 * @brief Project node runtime.
 *
 * Owns the node, its context, and a dedicated executor thread.
 */
class node_runtime {
public:
    /**
     * @brief Construct the node runtime.
     * @param node_name Node name.
     * @param options Node options.
     */
    explicit node_runtime(
        const std::string& node_name,
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~node_runtime();

    /** @brief Start the node runtime. */
    void start();

    /** @brief Stop the node runtime. */
    void stop();

    /** @brief Get the node. */
    std::shared_ptr<node> get_node() const;

    /** @brief Get the node context. */
    std::shared_ptr<node_context> get_node_context() const;

private:
    RCLCPP_DISABLE_COPY(node_runtime)

    std::shared_ptr<node> m_node;
    std::shared_ptr<node_context> m_node_context;

    rclcpp::executors::SingleThreadedExecutor m_executor;
    std::thread m_spin_thread;
    bool m_started = false;
};

}  // namespace clover2_common
