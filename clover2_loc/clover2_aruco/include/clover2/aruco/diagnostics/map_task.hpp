#pragma once

// ros2
#include <diagnostic_updater/diagnostic_updater.hpp>

// STL
#include <string>

namespace clover2::aruco::diagnostic {

class map : public diagnostic_updater::DiagnosticTask {
public:
    map()
        : diagnostic_updater::DiagnosticTask("map") {}

    void set_map_data(bool valid, const std::string& name, size_t count,
                      const std::string& frame);

    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& status) override;

    bool map_valid{false};
    std::string map_name;
    size_t map_count{0};
    std::string map_frame;
};

}  // namespace clover2::aruco::diagnostic
