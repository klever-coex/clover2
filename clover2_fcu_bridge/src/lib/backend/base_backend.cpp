#include <clover2/fcu_bridge/backend/base_backend.hpp>

namespace clover2::fcu_bridge::backend {

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

}  // namespace clover2::fcu_bridge::backend
