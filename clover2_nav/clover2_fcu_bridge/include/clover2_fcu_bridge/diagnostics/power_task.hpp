#pragma once

// clover2
#include <clover2_fcu_bridge/backend/base_backend.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_fcu_bridge::diagnostics {

class power_task : public diagnostic_updater::DiagnosticTask {
public:
    explicit power_task(const std::string& name = "power");

    void set_backend(const std::shared_ptr<backend::base_backend>& backend);
    void set_thresholds(double warn_percentage, double error_percentage);
    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    std::weak_ptr<backend::base_backend> m_backend;
    double m_warn_percentage{0.20};
    double m_error_percentage{0.10};
};

}  // namespace clover2_fcu_bridge::diagnostics