#include <clover2_common/node_interfaces/node_diagnostics.hpp>

namespace clover2_common::node_interfaces {

NodeDiagnostics::NodeDiagnostics(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_interface,
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
        parameters_interface,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr timers_interface,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr topics_interface)
    : m_updater(std::make_shared<diagnostic_updater::Updater>(
          base_interface, clock_interface, logging_interface,
          parameters_interface, timers_interface, topics_interface)) {
    m_updater->setHardwareID(base_interface->get_name());
}

NodeDiagnostics::~NodeDiagnostics() = default;

void NodeDiagnostics::add(diagnostic_updater::DiagnosticTask& task) {
    m_updater->add(task);
}

void NodeDiagnostics::remove_by_name(const std::string& name) {
    m_updater->removeByName(name);
}

void NodeDiagnostics::force_update() { m_updater->force_update(); }

}  // namespace clover2_common::node_interfaces
