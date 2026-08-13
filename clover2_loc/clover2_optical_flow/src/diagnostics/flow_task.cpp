#include <clover2/optical_flow/diagnostics/flow_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

// STL
#include <string>

namespace clover2::optical_flow::diagnostics {

flow_task::flow_task(const std::string& name)
    : diagnostic_updater::DiagnosticTask(name) {}

void flow_task::set_clock(rclcpp::Clock::SharedPtr clock) { m_clock = clock; }

void flow_task::update_required_tf(bool ok) { m_required_tf_ok = ok; }

void flow_task::update_flow(const rclcpp::Time& stamp, double quality) {
    m_last_flow_stamp = stamp;
    m_last_quality = quality;
}

void flow_task::reset() {
    m_required_tf_ok = true;
    m_last_flow_stamp = rclcpp::Time();
    m_last_quality = 0.0;
}

void flow_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_required_tf_ok) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Required TF transform failed");
    } else if (m_last_flow_stamp.nanoseconds() == 0) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Waiting for optical flow output");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Optical flow published");
    }

    stat.add("Required TF ok", m_required_tf_ok ? "true" : "false");
    stat.add("Last quality", m_last_quality);

    if (m_last_flow_stamp.nanoseconds() == 0) {
        stat.add("Last flow publish age, sec", "never");
    } else if (m_clock) {
        stat.add("Last flow publish age, sec",
                 (m_clock->now() - m_last_flow_stamp).seconds());
    } else {
        stat.add("Last flow publish age, sec", "unknown");
    }
}

}  // namespace clover2::optical_flow::diagnostics
