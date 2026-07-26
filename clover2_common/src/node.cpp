
// clover2
#include <clover2_common/node.hpp>

// ROS2
#include <rclcpp/node.hpp>

// STL
#include <memory>
#include <utility>

namespace clover2_common {

node::node(const std::string& node_name, const rclcpp::NodeOptions& options)
    : node(node_name, options, NodeInterfacesFactory<>{}) {}

clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
node::get_node_diagnostics_interface() {
    return m_diagnostics;
}

clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
node::get_node_parameters_watcher_interface() {
    return m_parameters_watcher;
}

}  // namespace clover2_common
