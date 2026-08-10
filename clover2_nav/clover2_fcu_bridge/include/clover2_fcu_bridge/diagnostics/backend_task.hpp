#pragma once

// clover2
#include <clover2_fcu_bridge/backend/base_backend.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_fcu_bridge::diagnostics {

class backend_task : public diagnostic_updater::DiagnosticTask {
public:
    explicit backend_task(const std::string& name = "backend");

    void set_backend(const std::shared_ptr<backend::base_backend>& backend);
    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    std::weak_ptr<backend::base_backend> m_backend;
};

}  // namespace clover2_fcu_bridge::diagnostics