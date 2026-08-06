#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics_interface.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>
#include <clover2_common/util/parameter.hpp>

// ROS2
#include <diagnostic_updater/diagnostic_updater.hpp>
#include <rclcpp/macros.hpp>
#include <rclcpp/parameter.hpp>
#include <rclcpp/rclcpp.hpp>

namespace clover2_common {

class node : public rclcpp::Node {
public:
    node(const std::string& node_name,
         const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    virtual ~node() = default;

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
    rclcpp::TimerBase::SharedPtr m_init_timer;

private:
    RCLCPP_DISABLE_COPY(node)

    // Custom node interfaces
    clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
        m_diagnostics;
    clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
        m_parameters_watcher;
};

}  // namespace clover2_common
