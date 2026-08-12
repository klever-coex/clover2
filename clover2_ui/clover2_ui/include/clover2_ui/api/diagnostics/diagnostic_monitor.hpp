#pragma once

#include <clover2_common/node_context.hpp>
#include <clover2_ui/api/diagnostics/diagnostic_model.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace clover2_ui::api::diagnostics {

class monitor {
public:
    explicit monitor(
        std::shared_ptr<clover2_common::node_context> node_context,
        const std::string& topic = "/diagnostics_agg");

    snapshot get_snapshot() const;
    const std::string& topic() const noexcept { return m_topic; }

private:
    void on_diagnostics(diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);

    std::string m_topic;
    std::shared_ptr<clover2_common::node_context> m_node_context;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        m_sub;

    mutable std::mutex m_mutex;
    model m_model;
    rclcpp::Time m_last_update;
};

}  // namespace clover2_ui::api::diagnostics
