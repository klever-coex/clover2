
// clover2
#include <clover2_common/node.hpp>
#include <clover2_common/node_interfaces/node_diagnostics.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher.hpp>

// ROS2
#include <rclcpp/node.hpp>

// STL
#include <memory>
#include <utility>

namespace clover2_common {

node::node(const std::string& node_name, const rclcpp::NodeOptions& options)
    : rclcpp::Node(node_name, options)
    , m_diagnostics(std::make_shared<node_interfaces::NodeDiagnostics>(
          get_node_base_interface(), get_node_clock_interface(),
          get_node_logging_interface(), get_node_parameters_interface(),
          get_node_timers_interface(), get_node_topics_interface()))
    , m_parameters_watcher(new node_interfaces::NodeParametersWatcher(
          get_node_parameters_interface())) {}

node::node(const std::string& node_name, const rclcpp::NodeOptions& options,
           clover2_common::node_interfaces::NodeDiagnosticsFactory::SharedPtr
               diagnostics_factory)
    : rclcpp::Node(node_name, options)
    , m_diagnostics(diagnostics_factory->create(
          get_node_base_interface(), get_node_clock_interface(),
          get_node_logging_interface(), get_node_parameters_interface(),
          get_node_timers_interface(), get_node_topics_interface()))
    , m_parameters_watcher(new node_interfaces::NodeParametersWatcher(
          get_node_parameters_interface())) {}

clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
node::get_node_diagnostics_interface() {
    return m_diagnostics;
}

clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
node::get_node_parameters_watcher_interface() {
    return m_parameters_watcher;
}

}  // namespace clover2_common
