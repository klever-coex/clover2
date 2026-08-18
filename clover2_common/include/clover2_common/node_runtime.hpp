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

class node_runtime {
public:
    explicit node_runtime(
        const std::string& node_name,
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~node_runtime();

    void start();
    void stop();

    std::shared_ptr<node> get_node() const;
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
