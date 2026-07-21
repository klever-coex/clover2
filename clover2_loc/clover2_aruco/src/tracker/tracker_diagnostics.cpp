#include <clover2/aruco/tracker_diagnostics.hpp>

namespace clover2::aruco {

const std::unordered_map<TrackerDiagnostics::diagnostic, std::string>
    TrackerDiagnostics::diagnostic_names = {
        {diagnostic::map, "map"},
        {diagnostic::markers, "markers"},
        {diagnostic::pose, "pose"},
        {diagnostic::pose_frequency, "pose_frequency"},
};

TrackerDiagnostics::TrackerDiagnostics(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_interface,
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
        parameters_interface,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr timers_interface,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr topics_interface)
    : clover2_common::node_interfaces::NodeDiagnostics(
          base_interface, clock_interface, logging_interface,
          parameters_interface, timers_interface, topics_interface) {
    for (const auto& [code, name] : diagnostic_names) {
        add(name,
            [this, code](diagnostic_updater::DiagnosticStatusWrapper& stat) {
                if (!apply_diagnostic_callback(code, stat)) {
                    stat.summary(diagnostic_msgs::msg::DiagnosticStatus::STALE,
                                 "Callback is not set");
                }
            });
    }
}

TrackerDiagnostics::~TrackerDiagnostics() = default;

void TrackerDiagnostics::set_diagnostic_callback(
    diagnostic diagnostic_code, callback callback) {
    m_diagnostic_callbacks[diagnostic_code] = callback;
}

void TrackerDiagnostics::remove_diagnostic_callback(
    diagnostic diagnostic_code) {
    m_diagnostic_callbacks.erase(diagnostic_code);
}

bool TrackerDiagnostics::apply_diagnostic_callback(
    diagnostic diagnostic_code,
    diagnostic_updater::DiagnosticStatusWrapper& status) const {
    const auto callback_it = m_diagnostic_callbacks.find(diagnostic_code);
    if (callback_it == m_diagnostic_callbacks.end()) {
        return false;
    }

    callback_it->second(status);
    return true;
}

}  // namespace clover2::aruco
