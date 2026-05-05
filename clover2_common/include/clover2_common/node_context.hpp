#pragma once

#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_interfaces.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_time_source_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/node_interfaces/node_type_descriptions_interface.hpp>
#include <rclcpp/node_interfaces/node_waitables_interface.hpp>
#include <rclcpp/rclcpp.hpp>

#include <string>

namespace clover2_common {

using namespace rclcpp::node_interfaces;
class node_context : public NodeInterfaces<NodeBaseInterface,              //
                                           NodeClockInterface,             //
                                           NodeGraphInterface,             //
                                           NodeLoggingInterface,           //
                                           NodeParametersInterface,        //
                                           NodeServicesInterface,          //
                                           NodeTimeSourceInterface,        //
                                           NodeTimersInterface,            //
                                           NodeTopicsInterface,            //
                                           NodeTypeDescriptionsInterface,  //
                                           NodeWaitablesInterface> {
public:
    template <typename NodeT>
    explicit node_context(NodeT& node)
        : NodeInterfaces(node) {}
};

}  // namespace clover2_common
