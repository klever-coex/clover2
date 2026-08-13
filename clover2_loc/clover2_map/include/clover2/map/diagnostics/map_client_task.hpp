#pragma once

// clover2
#include <clover2/map/client.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>

// STL
#include <memory>
#include <string>

namespace clover2::map::diagnostics {

class map_client_task : public diagnostic_updater::DiagnosticTask {
public:
    explicit map_client_task(const std::string& name = "map/client");

    void set_client(const std::shared_ptr<clover2::map::client>& client);

    void clear_client();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    std::weak_ptr<clover2::map::client> m_client;
};

}  // namespace clover2::map::diagnostics
