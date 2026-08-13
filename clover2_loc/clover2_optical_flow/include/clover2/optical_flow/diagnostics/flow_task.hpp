#pragma once

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/clock.hpp>
#include <rclcpp/time.hpp>

// STL
#include <string>

namespace clover2::optical_flow::diagnostics {

class flow_task : public diagnostic_updater::DiagnosticTask {
public:
    explicit flow_task(const std::string& name = "/sensors/camera/front/optical_flow/flow");

    void set_clock(rclcpp::Clock::SharedPtr clock);
    void update_required_tf(bool ok);
    void update_flow(const rclcpp::Time& stamp, double quality);
    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    rclcpp::Clock::SharedPtr m_clock;
    bool m_required_tf_ok{true};
    rclcpp::Time m_last_flow_stamp;
    double m_last_quality{0.0};
};
}  // namespace clover2::optical_flow::diagnostics
