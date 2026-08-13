#pragma once

// clover2
#include <clover2/map/io/fs_provider.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>

// STL
#include <memory>
#include <string>

namespace clover2::map::diagnostics {

class map_server_task : public diagnostic_updater::DiagnosticTask {
public:
    explicit map_server_task(
        const std::string& name = "/localization/map_server/map");

    void set_provider(const std::shared_ptr<io::fs_provider>& provider);
    void set_map_path(const std::string& path);
    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    std::weak_ptr<io::fs_provider> m_provider;
    std::string m_map_path;
};

}  // namespace clover2::map::diagnostics