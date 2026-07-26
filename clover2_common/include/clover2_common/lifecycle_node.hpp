#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics_interface.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>
#include <clover2_common/node_interfaces_factory.hpp>
#include <clover2_common/util/parameter.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/rclcpp.hpp>
#include <rclcpp_lifecycle/lifecycle_node.hpp>

// STL
#include <memory>
#include <string>

namespace clover2_common {

using CallbackReturn =
    rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

class lifecycle_node : public rclcpp_lifecycle::LifecycleNode {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(lifecycle_node)

    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    lifecycle_node(const std::string& node_name,
                   const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    template <typename NodeInterfacesFactoryT>
    lifecycle_node(const std::string& node_name,
                   const rclcpp::NodeOptions& options, NodeInterfacesFactoryT)
        : rclcpp_lifecycle::LifecycleNode(node_name, options)
        , m_diagnostics(NodeInterfacesFactoryT::create_diagnostics(
              get_node_base_interface(), get_node_clock_interface(),
              get_node_logging_interface(), get_node_parameters_interface(),
              get_node_timers_interface(), get_node_topics_interface()))
        , m_parameters_watcher(
              NodeInterfacesFactoryT::create_parameters_watcher(
                  get_node_parameters_interface())) {
        init_lifecycle_node();
    }

    virtual ~lifecycle_node();

    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        clover2_common::util::ParameterFunctorT cb,
        const std::string& description = "",
        const std::string& additional_constraints = "", bool read_only = false,
        bool ignore_override = false) {
        auto parameters_watcher = get_node_parameters_watcher_interface();

        clover2_common::util::declare_and_watch_parameter<ParameterT>(
            parameters_watcher, name, default_value, cb, description,
            additional_constraints, read_only, ignore_override);
    }

    // Custom node interfaces getters
    clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
    get_node_diagnostics_interface();

    clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
    get_node_parameters_watcher_interface();

protected:
    void produce_lifecycle_diagnostics(
        diagnostic_updater::DiagnosticStatusWrapper& status);

    rclcpp::TimerBase::SharedPtr m_init_timer;

private:
    RCLCPP_DISABLE_COPY(lifecycle_node)

    void init_lifecycle_node();

    // Custom node interfaces
    clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
        m_diagnostics;
    clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
        m_parameters_watcher;
};

}  // namespace clover2_common
