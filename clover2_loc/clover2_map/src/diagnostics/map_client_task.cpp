#include <clover2_map/diagnostics/map_client_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

// STL
#include <string>

namespace clover2_map::diagnostics {

map_client_task::map_client_task(const std::string& name)
    : diagnostic_updater::DiagnosticTask(name) {}

void map_client_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    const bool map_valid = m_map_valid_getter ? m_map_valid_getter() : false;

    stat.summary(map_valid ? diagnostic_msgs::msg::DiagnosticStatus::OK
                           : diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                 map_valid ? "Map valid" : "Map invalid or missing");

    if (map_valid) {
        stat.add("Map name", m_name_getter ? m_name_getter() : "unknown");
        stat.add("Map frame",
                 m_frame_id_getter ? m_frame_id_getter() : "unknown");
        stat.add("Marker count", m_marker_count_getter
                                     ? std::to_string(m_marker_count_getter())
                                     : "unknown");
    }
}

}  // namespace clover2_map::diagnostics
