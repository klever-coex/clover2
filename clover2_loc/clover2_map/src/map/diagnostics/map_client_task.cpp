#include <clover2/map/diagnostics/map_client_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

// STL
#include <string>

namespace clover2::map::diagnostics {

map_client_task::map_client_task(const std::string& name)
    : diagnostic_updater::DiagnosticTask(name) {}

void map_client_task::set_client(
    const std::shared_ptr<clover2::map::client>& client) {
    m_client = client;
}

void map_client_task::clear_client() { m_client.reset(); }

void map_client_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    auto client = m_client.lock();
    const bool map_valid = client && client->valid();

    stat.summary(map_valid ? diagnostic_msgs::msg::DiagnosticStatus::OK
                           : diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                 map_valid ? "Map valid" : "Map invalid or missing");

    stat.add("Map name", map_valid ? client->get_name() : "unknown");
    stat.add("Map frame", map_valid ? client->get_map_id() : "unknown");
    stat.add("Marker count",
             map_valid ? std::to_string(client->get_count()) : "0");
}

}  // namespace clover2::map::diagnostics