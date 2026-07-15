#include <clover2_fcu_bridge/server_diagnostics.hpp>

namespace clover2_fcu_bridge {

ServerDiagnostics::ServerDiagnostics(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_interface,
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
        parameters_interface,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr timers_interface,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr topics_interface)
    : clover2_common::node_interfaces::NodeDiagnostics(
          base_interface, clock_interface, logging_interface,
          parameters_interface, timers_interface, topics_interface)
    , m_diagnostic_names({{diagnostic::backend, "backend"},
                          {diagnostic::interfaces, "interfaces"},
                          {diagnostic::navigation, "navigation"}}) {
    for (const auto& [code, name] : m_diagnostic_names) {
        add(name,
            [this, code](diagnostic_updater::DiagnosticStatusWrapper& stat) {
                if (!apply_diagnostic_callback(code, stat)) {
                    stat.summary(diagnostic_msgs::msg::DiagnosticStatus::STALE,
                                 "Callback is not set");
                }
            });
    }
}

ServerDiagnostics::~ServerDiagnostics() = default;

void ServerDiagnostics::set_diagnostic_callback(diagnostic diagnostic_code,
                                                callback callback) {
    m_diagnostic_callbacks[diagnostic_code] = callback;
}

void ServerDiagnostics::remove_diagnostic_callback(diagnostic diagnostic_code) {
    m_diagnostic_callbacks.erase(diagnostic_code);
}

bool ServerDiagnostics::apply_diagnostic_callback(
    diagnostic diagnostic_code,
    diagnostic_updater::DiagnosticStatusWrapper& status) const {
    const auto callback_it = m_diagnostic_callbacks.find(diagnostic_code);
    if (callback_it == m_diagnostic_callbacks.end()) {
        return false;
    }

    callback_it->second(status);
    return true;
}

}  // namespace clover2_fcu_bridge