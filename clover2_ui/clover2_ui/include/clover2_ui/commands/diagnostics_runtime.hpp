#pragma once

#include <clover2_common/node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_ui/api/diagnostics/diagnostic_monitor.hpp>
#include <rclcpp/executors/single_threaded_executor.hpp>

#include <memory>
#include <string>
#include <thread>

namespace clover2_ui::commands {

class diagnostics_runtime {
public:
    explicit diagnostics_runtime(const std::string& topic = "/diagnostics_agg");
    ~diagnostics_runtime();

    diagnostics_runtime(const diagnostics_runtime&) = delete;
    diagnostics_runtime& operator=(const diagnostics_runtime&) = delete;

    void start();
    void stop();

    std::shared_ptr<api::diagnostics::diagnostic_monitor> monitor() const;

private:
    std::shared_ptr<clover2_common::node> m_node;
    std::shared_ptr<clover2_common::node_context> m_node_context;
    std::shared_ptr<api::diagnostics::diagnostic_monitor> m_monitor;

    rclcpp::executors::SingleThreadedExecutor m_executor;
    std::thread m_spin_thread;
    bool m_started;
};

}  // namespace clover2_ui::commands
