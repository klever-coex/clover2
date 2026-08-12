#pragma once

// clover2
#include <clover2_fcu_bridge/backend/base_backend.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>

// STL
#include <chrono>
#include <memory>
#include <string>

namespace clover2_fcu_bridge::diagnostics {

class barometer_task : public diagnostic_updater::DiagnosticTask {
public:
    explicit barometer_task(const std::string& name = "barometer");

    void set_backend(const std::shared_ptr<backend::base_backend>& backend);
    void set_clock(const rclcpp::Clock::SharedPtr& clock);
    void set_stale_timeout(std::chrono::duration<double> timeout);
    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    std::weak_ptr<backend::base_backend> m_backend;
    rclcpp::Clock::SharedPtr m_clock;
    std::chrono::duration<double> m_stale_timeout{1.0};
};

}  // namespace clover2_fcu_bridge::diagnostics
