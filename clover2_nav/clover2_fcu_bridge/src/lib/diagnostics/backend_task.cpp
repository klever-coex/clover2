#include <clover2_fcu_bridge/diagnostics/backend_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

namespace clover2_fcu_bridge::diagnostics {

backend_task::backend_task(const std::string& name)
    : diagnostic_updater::DiagnosticTask(name) {}

void backend_task::set_backend(
    const std::shared_ptr<backend::base_backend>& backend) {
    m_backend = backend;
}

void backend_task::reset() { m_backend.reset(); }

void backend_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    auto backend = m_backend.lock();

    if (!backend) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Backend is not initialized");
        return;
    }

    const auto state = backend->get_fcu_state();

    if (!state.value) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "FCU state is not received");
        return;
    } else if (!state.value->connected) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "FCU is not connected");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "FCU connected");
    }

    stat.add("Connected", state.value->connected ? "true" : "false");
    stat.add("Armed", state.value->armed ? "true" : "false");
    stat.add("Mode", state.value->flight_mode.to_str());
}

}  // namespace clover2_fcu_bridge::diagnostics