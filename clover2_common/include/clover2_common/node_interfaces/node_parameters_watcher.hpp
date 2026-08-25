/**
 * @file node_parameters_watcher.hpp
 * @brief Provides project node parameters watcher.
 */

#pragma once

// clover2
#include <clover2_common/node_interfaces/node_parameters_watcher_interface.hpp>

// ROS2
#include <rclcpp/macros.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/visibility_control.hpp>

// STL
#include <memory>

namespace clover2_common::node_interfaces {

/**
 * @brief Project node parameters watcher.
 */
class NodeParametersWatcher : public NodeParametersWatcherInterface {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(NodeParametersWatcher)

    /**
     * @brief Construct node parameters watcher.
     * @param node_parameters Node parameters interface.
     */
    RCLCPP_PUBLIC
    explicit NodeParametersWatcher(
        const rclcpp::node_interfaces::NodeParametersInterface::SharedPtr&
            node_parameters);

    RCLCPP_PUBLIC
    virtual ~NodeParametersWatcher();

    /**
     * @brief Declare a parameter and invoke the callback when it changes.
     *
     * @param name Parameter name.
     * @param default_value Default value.
     * @param cb On change callback.
     * @param parameter_descriptor Parameter descriptor.
     * @param ignore_override Ignore parameter override.
     */
    RCLCPP_PUBLIC
    void declare_and_watch_parameter(
        const std::string& name, const rclcpp::ParameterValue& default_value,
        ParameterFunctorT cb,
        const rcl_interfaces::msg::ParameterDescriptor& parameter_descriptor =
            rcl_interfaces::msg::ParameterDescriptor(),
        bool ignore_override = false) override final;

    /** @brief Undeclare all watched parameters. */
    RCLCPP_PUBLIC
    void undeclare_watcher_parameters() override final;

private:
    RCLCPP_DISABLE_COPY(NodeParametersWatcher)

    class NodeParametersWatcherImpl;
    std::unique_ptr<NodeParametersWatcherImpl> m_impl;
};

}  // namespace clover2_common::node_interfaces
