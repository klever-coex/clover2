#include <clover2_map/diagnostics/map_server_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

// STL
#include <string>

namespace clover2_map::diagnostics {

map_server_task::map_server_task(const std::string& name)
    : diagnostic_updater::DiagnosticTask(name) {}

void map_server_task::set_provider(
    const std::shared_ptr<io::fs_provider>& provider) {
    m_provider = provider;
}

void map_server_task::set_map_path(const std::string& path) {
    m_map_path = path;
}

void map_server_task::reset() {
    m_provider.reset();
    m_map_path.clear();
}

void map_server_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    auto provider = m_provider.lock();

    if (!provider) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Map not loaded");

        stat.add("Map frame", "unknown");
        stat.add("Map path", m_map_path.empty() ? "unknown" : m_map_path);
        stat.add("Marker count", "0");
        return;
    }

    const auto& map = provider->get_map();

    if (map.markers.empty()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Map loaded but empty");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK, "Map loaded");
    }

    stat.add("Map frame", map.frame_id.empty() ? "unknown" : map.frame_id);
    stat.add("Map path", m_map_path.empty() ? "unknown" : m_map_path);
    stat.add("Marker count", std::to_string(map.markers.size()));
}

}  // namespace clover2_map::diagnostics
