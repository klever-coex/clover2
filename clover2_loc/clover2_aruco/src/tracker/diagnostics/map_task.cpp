#include <clover2/aruco/diagnostics/map_task.hpp>

namespace clover2::aruco::diagnostics {

void map_task::set_map_data(bool valid, const std::string& name, size_t count,
                            const std::string& frame) {
    m_map_valid = valid;
    m_map_name = name;
    m_map_count = count;
    m_map_frame = frame;
}

void map_task::reset() {
    m_map_valid = false;
    m_map_name.clear();
    m_map_count = 0;
    m_map_frame.clear();
}

void map_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_map_valid) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Map is invalid");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Map is valid");
    }

    stat.add("Map name", m_map_valid ? m_map_name : "unknown");
    stat.add("Map frame", m_map_valid ? m_map_frame : "unknown");
    stat.add("Marker count", m_map_valid ? std::to_string(m_map_count) : "0");
}
}  // namespace clover2::aruco::diagnostics
