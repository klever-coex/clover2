#include <clover2_fcu_bridge/diagnostics/imu_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

namespace clover2_fcu_bridge::diagnostics {

imu_task::imu_task(const std::string& name)
    : diagnostic_updater::DiagnosticTask(name) {}

void imu_task::set_backend(
    const std::shared_ptr<backend::base_backend>& backend) {
    m_backend = backend;
}

void imu_task::set_clock(const rclcpp::Clock::SharedPtr& clock) {
    m_clock = clock;
}

void imu_task::set_stale_timeout(std::chrono::duration<double> timeout) {
    m_stale_timeout = timeout;
}

void imu_task::reset() { m_backend.reset(); }

void imu_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    const auto backend = m_backend.lock();
    const auto data = backend ? backend->get_imu()
                              : std::optional<sensor_msgs::msg::Imu>{};
    if (!backend || !data) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     backend ? "IMU data is not received"
                             : "Backend is not initialized");
        return;
    }

    const double age =
        m_clock ? (m_clock->now() - rclcpp::Time(data->header.stamp)).seconds()
                : m_stale_timeout.count();
    if (age > m_stale_timeout.count()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "IMU data is stale");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "IMU data is fresh");
    }
    stat.add("Data age, sec", age);
}

}  // namespace clover2_fcu_bridge::diagnostics
