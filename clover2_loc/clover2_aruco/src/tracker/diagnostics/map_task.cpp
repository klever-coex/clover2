#include <clover2/aruco/diagnostics/map_task.hpp>

namespace clover2::aruco::diagnostic {

void map::set_map_data(bool valid, const std::string& name, size_t count,
                       const std::string& frame) {
    map_valid = valid;
    map_name = name;
    map_count = count;
    map_frame = frame;
}

void map::reset() {
    map_valid = false;
    map_name.clear();
    map_count = 0;
    map_frame.clear();
}

void map::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!map_valid) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Map is invalid");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Map is valid");
    }

    stat.add("Map name", map_valid ? map_name : "unknown");
    stat.add("Map frame", map_valid ? map_frame : "unknown");
    stat.add("Marker count", map_valid ? std::to_string(map_count) : "0");
}
}  // namespace clover2::aruco::diagnostic
