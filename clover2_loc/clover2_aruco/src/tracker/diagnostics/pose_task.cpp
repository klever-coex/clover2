#include <clover2/aruco/diagnostics/pose_task.hpp>

// STL
#include <string>

namespace clover2::aruco::diagnostics {

namespace {

bool same_pose(const geometry_msgs::msg::Pose& lhs,
               const geometry_msgs::msg::Pose& rhs) {
    return lhs.position.x == rhs.position.x &&
           lhs.position.y == rhs.position.y &&
           lhs.position.z == rhs.position.z &&
           lhs.orientation.x == rhs.orientation.x &&
           lhs.orientation.y == rhs.orientation.y &&
           lhs.orientation.z == rhs.orientation.z &&
           lhs.orientation.w == rhs.orientation.w;
}

}  // namespace

void pose_task::set_clock(rclcpp::Clock::SharedPtr clock) { m_clock = clock; }

void pose_task::update_pose(const rclcpp::Time& stamp,
                            const geometry_msgs::msg::Pose& pose) {
    if (m_last_pose_stamp.nanoseconds() == 0) {
        m_pose_changed = true;
    } else {
        m_pose_changed = !same_pose(m_last_pose, pose);
    }

    m_last_pose = pose;
    m_last_pose_stamp = stamp;
}

void pose_task::reset() {
    m_last_pose_stamp = rclcpp::Time();
    m_last_pose = geometry_msgs::msg::Pose();
    m_pose_changed = false;
}

void pose_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (m_last_pose_stamp.nanoseconds() == 0) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Waiting for pose output");
    } else if (!m_pose_changed) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Pose is not changing");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Pose is changing");
    }

    if (m_last_pose_stamp.nanoseconds() == 0) {
        stat.add("Last pose publish age, sec", "never");
    } else if (m_clock) {
        stat.add("Last pose publish age, sec",
                 (m_clock->now() - m_last_pose_stamp).seconds());
    } else {
        stat.add("Last pose publish age, sec", "unknown");
    }

    stat.add("Pose changed", m_pose_changed ? "true" : "false");
    stat.add("Last position x", m_last_pose.position.x);
    stat.add("Last position y", m_last_pose.position.y);
    stat.add("Last position z", m_last_pose.position.z);
    stat.add("Last orientation x", m_last_pose.orientation.x);
    stat.add("Last orientation y", m_last_pose.orientation.y);
    stat.add("Last orientation z", m_last_pose.orientation.z);
    stat.add("Last orientation w", m_last_pose.orientation.w);
}

}  // namespace clover2::aruco::diagnostics