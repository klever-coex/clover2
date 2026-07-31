#include <clover2_ui/api/diagnostics/diagnostic_monitor.hpp>

namespace clover2_ui::api::diagnostics {

diagnostic_monitor::diagnostic_monitor(const std::string& topic)
    : m_topic(topic)
    , m_node(
          std::make_shared<clover2_common::node>("clover2_diagnostics_tui")) {
    m_sub = m_node->create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
        m_topic, rclcpp::QoS(10),
        [this](diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg) {
            on_diagnostics(std::move(msg));
        });
}

void diagnostic_monitor::on_diagnostics(
    diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_model.update(*msg);
    m_last_update = m_node->now();
}

diagnostic_snapshot diagnostic_monitor::snapshot() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto out = m_model.snapshot();
    if (out.received) {
        out.age_sec = (m_node->now() - m_last_update).seconds();
    }
    return out;
}

}  // namespace clover2_ui::api::diagnostics
