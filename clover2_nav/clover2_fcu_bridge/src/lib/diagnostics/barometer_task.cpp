#include <clover2_fcu_bridge/diagnostics/barometer_task.hpp>

// ROS2
#include <diagnostic_msgs/msg/diagnostic_status.hpp>

namespace clover2_fcu_bridge::diagnostics {

barometer_task::barometer_task(const std::string& name)
    : diagnostic_updater::DiagnosticTask(name) {}

void barometer_task::set_backend(
    const std::shared_ptr<backend::base_backend>& backend) {
    m_backend = backend;
}

void barometer_task::set_clock(const rclcpp::Clock::SharedPtr& clock) {
    m_clock = clock;
}

void barometer_task::set_stale_timeout(std::chrono::duration<double> timeout) {
    m_stale_timeout = timeout;
}

void barometer_task::reset() { m_backend.reset(); }

void barometer_task::run(diagnostic_updater::DiagnosticStatusWrapper& stat) {
    const auto backend = m_backend.lock();
    const auto data =
        backend ? backend->get_barometer()
                : std::optional<sensor_msgs::msg::FluidPressure>{};
    if (!backend || !data) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     backend ? "Barometer data is not received"
                             : "Backend is not initialized");
        return;
    }

    const double age =
        m_clock ? (m_clock->now() - rclcpp::Time(data->header.stamp)).seconds()
                : m_stale_timeout.count();
    if (age > m_stale_timeout.count()) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::WARN,
                     "Barometer data is stale");
    } else {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::OK,
                     "Barometer data is fresh");
    }
    stat.add("Data age, sec", age);
}

}  // namespace clover2_fcu_bridge::diagnostics
