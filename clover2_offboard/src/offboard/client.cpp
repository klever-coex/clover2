// clover2
#include <clover2/offboard/client.hpp>

namespace clover2::offboard {

client::client(rclcpp::Node::SharedPtr node,
               std::shared_ptr<bridge::base_bridge> bridge)
    : m_node(std::move(node)), m_bridge(std::move(bridge)) {
    if (!m_bridge) {
        throw std::invalid_argument("client: bridge cannot be null");
    }
}

void client::set_offboard_controller(bridge::data::controller_type type) {
    m_bridge->set_offboard_controller(type);
}

void client::set_local_position_setpoint(const Eigen::Vector3d& position,
                                         double yaw) {
    m_bridge->set_local_position_setpoint(position, yaw);
}

void client::set_local_velocity_setpoint(const Eigen::Vector3d& velocity,
                                        double yaw_rate) {
    m_bridge->set_local_velocity_setpoint(velocity, yaw_rate);
}

void client::set_speed(double speed) {
    m_bridge->set_speed(speed);
}

void client::arm() {
    m_bridge->arm();
}

void client::disarm() {
    m_bridge->disarm();
}

Eigen::Vector3d client::get_local_position() const {
    return m_bridge->get_local_position();
}

Eigen::Quaterniond client::get_orientation() const {
    return m_bridge->get_orientation();
}

Eigen::Vector3d client::get_orientation_rpy() const {
    return m_bridge->get_orientation_rpy();
}

std::string client::get_mode() const {
    return m_bridge->get_mode();
}

}  // namespace clover2::offboard
