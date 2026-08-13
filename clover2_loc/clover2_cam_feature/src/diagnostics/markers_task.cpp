#include <clover2/cam_feature/diagnostics/markers_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

// STL
#include <string>

namespace clover2::cam_feature::diagnostics {

void markers_task::set_clock(rclcpp::Clock::SharedPtr clock) {
    m_clock = clock;
}

void markers_task::update_markers(const rclcpp::Time& stamp, size_t count) {
    m_last_markers_stamp = stamp;
    m_last_marker_count = count;
}

void markers_task::reset() {
    m_last_markers_stamp = rclcpp::Time();
    m_last_marker_count = 0;
}

void markers_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (m_last_markers_stamp.nanoseconds() == 0) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Waiting for marker publications");
    } else if (m_last_marker_count == 0) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "MarkerArray published, no visible markers");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "MarkerArray published");
    }

    stat.add("Last marker count", std::to_string(m_last_marker_count));

    if (m_last_markers_stamp.nanoseconds() == 0) {
        stat.add("Last publication age, sec", "never");
    } else if (m_clock) {
        stat.add("Last publication age, sec",
                 (m_clock->now() - m_last_markers_stamp).seconds());
    } else {
        stat.add("Last publication age, sec", "unknown");
    }
}

}  // namespace clover2::cam_feature::diagnostics
