#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics_interface.hpp>

// ROS2
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>

// STL
#include <memory>
#include <type_traits>

namespace clover2_common::node_interfaces {

class NodeDiagnosticsFactory {
public:
    using SharedPtr = std::shared_ptr<NodeDiagnosticsFactory>;

    virtual ~NodeDiagnosticsFactory() = default;

    virtual NodeDiagnosticsInterface::SharedPtr create(
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
        rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
        rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
            logging_interface,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            parameters_interface,
        rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
            timers_interface,
        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
            topics_interface) = 0;
};

//! Helper template to make node diagnostics factories.
template <typename DiagnosticsT>
class NodeDiagnosticsFactoryTemplate : public NodeDiagnosticsFactory {
public:
    NodeDiagnosticsFactoryTemplate() = default;
    ~NodeDiagnosticsFactoryTemplate() override = default;

    NodeDiagnosticsInterface::SharedPtr create(
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr base_interface,
        rclcpp::node_interfaces::NodeClockInterface::SharedPtr clock_interface,
        rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr
            logging_interface,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            parameters_interface,
        rclcpp::node_interfaces::NodeTimersInterface::SharedPtr
            timers_interface,
        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr
            topics_interface) override {
        static_assert(
            std::is_base_of<NodeDiagnosticsInterface, DiagnosticsT>::value,
            "DiagnosticsT should be derived from NodeDiagnosticsInterface");

        return std::make_shared<DiagnosticsT>(
            base_interface, clock_interface, logging_interface,
            parameters_interface, timers_interface, topics_interface);
    }
};

}  // namespace clover2_common::node_interfaces