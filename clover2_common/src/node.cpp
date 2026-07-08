
// clover2
#include <clover2_common/node.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher.hpp>

// ROS2
#include <rclcpp/node.hpp>

namespace clover2_common {

node::node(const std::string& node_name, const rclcpp::NodeOptions& options)
    : rclcpp::Node(node_name, options)
    , m_parameters_watcher(new node_interfaces::NodeParametersWatcher(
          get_node_parameters_interface())) {
    enable_diagnostic_updater();
}

void node::enable_diagnostic_updater() {
    m_diagnostic_updater = std::make_shared<diagnostic_updater::Updater>(this);
    m_diagnostic_updater->setHardwareID(this->get_name());
}

std::shared_ptr<diagnostic_updater::Updater> node::get_diagnostic_updater()
    const {
    return m_diagnostic_updater;
}

clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
node::get_node_parameters_watcher_interface() {
    return m_parameters_watcher;
}

}  // namespace clover2_common
