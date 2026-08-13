#pragma once

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/clock.hpp>
#include <rclcpp/time.hpp>

// STL
#include <memory>

namespace clover2::cam_feature::diagnostics {

class markers_task : public diagnostic_updater::DiagnosticTask {
public:
    markers_task()
        : diagnostic_updater::DiagnosticTask(
              "/sensors/camera/main/cam_feature/markers") {}

    void set_clock(rclcpp::Clock::SharedPtr clock);
    void update_markers(const rclcpp::Time& stamp, size_t count);
    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    rclcpp::Clock::SharedPtr m_clock;
    rclcpp::Time m_last_markers_stamp;
    size_t m_last_marker_count{0};
};

}  // namespace clover2::cam_feature::diagnostics
