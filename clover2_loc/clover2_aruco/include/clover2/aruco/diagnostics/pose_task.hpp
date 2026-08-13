#pragma once

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <geometry_msgs/msg/pose.hpp>
#include <rclcpp/clock.hpp>
#include <rclcpp/time.hpp>

// STL
#include <memory>

namespace clover2::aruco::diagnostics {

class pose_task : public diagnostic_updater::DiagnosticTask {
public:
    pose_task()
        : diagnostic_updater::DiagnosticTask("pose") {}

    void set_clock(rclcpp::Clock::SharedPtr clock);
    void update_pose(const rclcpp::Time& stamp,
                     const geometry_msgs::msg::Pose& pose);
    void reset();

private:
    void run(diagnostic_updater::DiagnosticStatusWrapper& stat) override;

    rclcpp::Clock::SharedPtr m_clock;
    rclcpp::Time m_last_pose_stamp;
    geometry_msgs::msg::Pose m_last_pose;
    bool m_pose_changed{false};
};

}  // namespace clover2::aruco::diagnostics