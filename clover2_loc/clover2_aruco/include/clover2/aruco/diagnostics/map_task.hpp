#pragma once

// ros2
#include <diagnostic_updater/diagnostic_updater.hpp>

// STL
#include <string>

namespace clover2::aruco::diagnostics {

class map_task : public diagnostic_updater::DiagnosticTask {
public:
    map_task()
        : diagnostic_updater::DiagnosticTask("Map") {}

    void set_map_data(bool valid, const std::string& name, size_t count,
                      const std::string& frame);

    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& status) override;

    bool m_map_valid{false};
    std::string m_map_name;
    size_t m_map_count{0};
    std::string m_map_frame;
};

}  // namespace clover2::aruco::diagnostics
