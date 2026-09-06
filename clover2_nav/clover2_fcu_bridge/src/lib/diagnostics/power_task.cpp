#include <clover2_fcu_bridge/diagnostics/power_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

#include <cmath>

namespace clover2_fcu_bridge::diagnostics {

power_task::power_task(const std::string& name)
    : diagnostic_updater::DiagnosticTask(name) {}

void power_task::set_backend(
    const std::shared_ptr<backend::base_backend>& backend) {
    m_backend = backend;
}

void power_task::set_thresholds(double warn_percentage,
                                double error_percentage) {
    m_warn_percentage = warn_percentage;
    m_error_percentage = error_percentage;
}

void power_task::reset() { m_backend.reset(); }

void power_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    const auto backend = m_backend.lock();
    if (!backend) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Backend is not initialized");
        return;
    }

    const auto power = backend->get_power();
    if (!power) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Battery data is not received");
        return;
    }

    if (!std::isfinite(power->percentage)) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Battery percentage is not available");
    } else if (power->percentage <= m_error_percentage) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Battery charge is critically low");
    } else if (power->percentage <= m_warn_percentage) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Battery charge is low");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Battery charge is normal");
    }

    stat.add("Voltage", power->voltage);
    stat.add("Percentage", power->percentage);
}

}  // namespace clover2_fcu_bridge::diagnostics
