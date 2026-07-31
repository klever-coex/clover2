#pragma once

#include <clover2_ui/api/diagnostics/diagnostic_model.hpp>
#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <rclcpp/rclcpp.hpp>

#include <memory>
#include <mutex>
#include <string>

namespace clover2_ui::api::diagnostics {

class diagnostic_monitor {
public:
    explicit diagnostic_monitor(const std::string& topic = "/diagnostics_agg");

    rclcpp::Node::SharedPtr node() const { return m_node; }
    diagnostic_snapshot snapshot() const;
    const std::string& topic() const noexcept { return m_topic; }

private:
    void on_diagnostics(diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg);

    std::string m_topic;
    rclcpp::Node::SharedPtr m_node;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        m_sub;

    mutable std::mutex m_mutex;
    diagnostic_model m_model;
    rclcpp::Time m_last_update;
};

}  // namespace clover2_ui::api::diagnostics
