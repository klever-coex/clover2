#include <clover2_common/node_interfaces/node_diagnostics.hpp>

namespace clover2_common::node_interfaces {

class NodeDiagnostics::NodeDiagnosticsImpl {
public:
    NodeDiagnosticsImpl(
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
        rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
        rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
            logging_interface,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            parameters_interface,
        rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
            timers_interface,
        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
            topics_interface)
        : m_updater(std::make_shared<diagnostic_updater::Updater>(
              base_interface, clock_interface, logging_interface,
              parameters_interface, timers_interface, topics_interface)) {
        m_updater->setHardwareID(base_interface->get_name());
    }

    void add(const std::string& name, DiagnosticTaskCallbackT callback) {
        m_updater->add(name, callback);
    }

    void remove_by_name(const std::string& name) {
        m_updater->removeByName(name);
    }

    void force_update() { m_updater->force_update(); }

private:
    std::shared_ptr<diagnostic_updater::Updater> m_updater;
};

NodeDiagnostics::NodeDiagnostics(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
    rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr logging_interface,
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
        parameters_interface,
    rclcpp::node_interfaces::NodeTimersInterface::SharedPtr timers_interface,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr topics_interface)
    : m_impl(new NodeDiagnosticsImpl(base_interface, clock_interface,
                                     logging_interface, parameters_interface,
                                     timers_interface, topics_interface)) {}

NodeDiagnostics::~NodeDiagnostics() {}

void NodeDiagnostics::add(const std::string& name,
                          DiagnosticTaskCallbackT callback) {
    m_impl->add(name, callback);
}

void NodeDiagnostics::remove_by_name(const std::string& name) {
    m_impl->remove_by_name(name);
}

void NodeDiagnostics::force_update() { m_impl->force_update(); }

}  // namespace clover2_common::node_interfaces
