#include <clover2_fcu_bridge/backend/base_backend.hpp>

namespace clover2_fcu_bridge::backend {

base_backend::base_backend(const context& ctx)
    : m_ctx(ctx)
    , m_logger(
          m_ctx.get_node_logging_interface()->get_logger().get_child("backend"))
    , m_clock(m_ctx.get_node_clock_interface()->get_clock()) {}

void base_backend::set_position_setpoint(double x, double y, double z,
                                         double yaw) {
    const std::optional<tf2::Vector3> p = tf2::Vector3(x, y, z);
    std::optional<double> yaw_op = yaw;

    set_setpoint(p, std::nullopt, std::nullopt, yaw_op, std::nullopt);
}

void base_backend::set_velocity_setpoint(double vx, double vy, double vz,
                                         double yaw_rate) {
    const std::optional<tf2::Vector3> v = tf2::Vector3(vx, vy, vz);
    std::optional<double> yaw_rate_op = yaw_rate;

    set_setpoint(std::nullopt, v, std::nullopt, std::nullopt, yaw_rate_op);
}

fcu_state_snapshot base_backend::get_fcu_state_snapshot() const {
    fcu_state_snapshot snapshot;
    snapshot.received = true;
    snapshot.connected = connected();
    snapshot.armed = is_armed();
    snapshot.mode = get_mode().to_str();
    return snapshot;
}

power_snapshot base_backend::get_power_snapshot() const {
    return power_snapshot{};
}

imu_snapshot base_backend::get_imu_snapshot() const {
    return imu_snapshot{};
}

barometer_snapshot base_backend::get_barometer_snapshot() const {
    return barometer_snapshot{};
}

}  // namespace clover2_fcu_bridge::backend
