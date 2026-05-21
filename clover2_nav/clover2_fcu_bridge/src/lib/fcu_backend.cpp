#include <clover2_fcu_bridge/fcu_backend.hpp>

#include <stdexcept>

namespace clover2_fcu_bridge {

fcu_backend::fcu_backend(std::weak_ptr<backend::base_backend> backend)
    : m_backend(std::move(backend)) {}

std::shared_ptr<backend::base_backend> fcu_backend::lock() const {
    auto b = m_backend.lock();
    if (!b) {
        throw std::runtime_error("fcu_backend: backend is gone");
    }
    return b;
}

bool fcu_backend::ready() const {
    auto b = m_backend.lock();
    return static_cast<bool>(b) && b->ready();
}

bool fcu_backend::connected() const {
    auto b = m_backend.lock();
    return static_cast<bool>(b) && b->connected();
}

bool fcu_backend::is_armed() const { return lock()->is_armed(); }

data::mode fcu_backend::get_mode() const { return lock()->get_mode(); }

void fcu_backend::arm() { lock()->arm(); }

void fcu_backend::disarm() { lock()->disarm(); }

void fcu_backend::set_mode(const data::mode& mode) { lock()->set_mode(mode); }

geometry_msgs::msg::PoseStamped fcu_backend::get_pose() const {
    return lock()->get_pose();
}

void fcu_backend::set_position_setpoint(double x, double y, double z,
                                        double yaw) {
    lock()->set_position_setpoint(x, y, z, yaw);
}

void fcu_backend::set_velocity_setpoint(double vx, double vy, double vz,
                                        double yaw_rate) {
    lock()->set_velocity_setpoint(vx, vy, vz, yaw_rate);
}

}  // namespace clover2_fcu_bridge
