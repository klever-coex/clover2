/**
 * @file node.hpp
 * @brief Provides project base node.
 */

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

/**
 * @brief Project base node.
 */
class node : public rclcpp::Node {
public:
    /**
     * @brief Construct the node.
     * @param node_name Node name.
     * @param options Node options.
     */
    node(const std::string& node_name,
         const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    virtual ~node() = default;

    /**
     * @brief Declare a parameter and invoke the callback when it changes.
     *
     * @tparam ParameterT Type of the parameter.
     *
     * @param name Parameter name.
     * @param default_value Default value.
     * @param cb On change callback.
     * @param description Parameter description.
     * @param additional_constraints Additional constraints for the descriptor.
     * @param read_only Makes parameter readonly.
     * @param ignore_override Ignore parameter override.
     */
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

    /** @brief Get the diagnostics interface. */
    clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
    get_node_diagnostics_interface();

    /** @brief Get the parameters watcher interface. */
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
