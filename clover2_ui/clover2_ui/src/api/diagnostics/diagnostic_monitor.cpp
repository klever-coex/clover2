#include <clover2_ui/api/diagnostics/diagnostic_monitor.hpp>

#include <stdexcept>
#include <utility>

namespace clover2_ui::api::diagnostics {

diagnostic_monitor::diagnostic_monitor(
    std::shared_ptr<clover2_common::node_context> node_context,
    const std::string& topic)
    : m_topic(topic)
    , m_node_context(std::move(node_context)) {
    if (!m_node_context) {
        throw std::invalid_argument("diagnostic_monitor: node_context is null");
    }

    m_sub = rclcpp::create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
        m_node_context, m_topic, rclcpp::QoS(10),
        [this](diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg) {
            on_diagnostics(std::move(msg));
        });
}

void diagnostic_monitor::on_diagnostics(
    diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_model.update(*msg);
    m_last_update =
        m_node_context->get_node_clock_interface()->get_clock()->now();
}

diagnostic_snapshot diagnostic_monitor::snapshot() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto out = m_model.snapshot();
    if (out.received) {
        const auto now =
            m_node_context->get_node_clock_interface()->get_clock()->now();
        out.age_sec = (now - m_last_update).seconds();
    }
    return out;
}

}  // namespace clover2_ui::api::diagnostics
