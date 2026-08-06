#pragma once

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp_lifecycle/state.hpp>

// STL
#include <functional>

namespace clover2_common::diagnostics {

class lifecycle_state_task : public diagnostic_updater::DiagnosticTask {
public:
    using state_getter_t = std::function<rclcpp_lifecycle::State()>;

    lifecycle_state_task()
        : diagnostic_updater::DiagnosticTask("Lifecycle State") {}

    void set_state_getter(state_getter_t getter);

protected:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

private:
    state_getter_t m_state_getter;
};

}  // namespace clover2_common::diagnostics
