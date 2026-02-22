#pragma once

// RCLCPP
#include <rclcpp/rclcpp.hpp>

// STL
#include <unordered_map>
#include <vector>

namespace clover2::common {

/**
 * @brief Utility class for automatic parameter declaration and change
 * monitoring.
 *
 * @details
 * parameter_watcher provides a lightweight, reusable mechanism to:
 *  - declare parameters
 *  - register callbacks for parameter updates
 *  - automatically subscribe to on_set_parameters events
 *  - automatically undeclare all owned parameters on destruction (RAII)
 *
 * The class is designed to work with both:
 *  - rclcpp::Node
 *  - rclcpp_lifecycle::LifecycleNode
 *
 * It uses composition rather than inheritance, allowing it to be embedded
 * inside any component or node.
 *
 * Typical usage:
 * @code
 * parameter_watcher watcher(*this);
 *
 * watcher.declare_and_watch_parameter<int>(
 *     "rate",
 *     10,
 *     [](const rclcpp::Parameter & p) {
 *         std::cout << p.as_int() << std::endl;
 *     });
 * @endcode
 *
 * Lifetime behavior:
 *  - constructor/enable_watch_parameters(): registers callback
 *  - declare_and_watch_parameter(): declares parameter + stores handler
 *  - destructor(): removes callback + undeclares parameters
 *
 * @note Not copyable (callback handle ownership).
 */
class parameter_watcher {
public:
    /// Smart pointer typedefs and deleted copy operations
    RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(parameter_watcher)

    /// Alias for ROS parameter update result type
    using SetParametersResult = rcl_interfaces::msg::SetParametersResult;

    /**
     * @brief Callback type invoked when a watched parameter changes.
     *
     * The callback receives the updated parameter instance.
     */
    using ParameterFunctorT = std::function<void(const rclcpp::Parameter&)>;

    /**
     * @brief Construct watcher and immediately attach it to a node.
     *
     * @tparam NodeT rclcpp::Node or rclcpp_lifecycle::LifecycleNode
     * @param node Node instance used to register parameter callbacks.
     *
     * Equivalent to default construction followed by enable_watch_parameters().
     */
    template <typename NodeT>
    parameter_watcher(NodeT& node) {
        enable_watch_parameters(node);
    }

    /**
     * @brief Default constructor.
     *
     * You must later call enable_watch_parameters() before declaring
     * parameters.
     */
    parameter_watcher() = default;

    /**
     * @brief Destructor.
     *
     * Automatically:
     *  - removes parameter callback handle
     *  - undeclares all parameters declared by this watcher
     */
    ~parameter_watcher() { cleanup(); }

    /**
     * @brief Declare a parameter and register a change callback.
     *
     * @tparam ParameterT Parameter value type (int, double, bool, string, etc.)
     *
     * @param name Parameter name.
     * @param default_value Default value used for declaration.
     * @param cb Callback invoked whenever the parameter changes.
     * @param description Human-readable description.
     * @param additional_constraints Additional constraint text.
     * @param read_only If true, parameter cannot be modified at runtime.
     * @param ignore_override Ignore externally provided overrides.
     *
     * @note enable_watch_parameters() must be called before this method.
     */
    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        ParameterFunctorT cb, const std::string& description = "",
        const std::string& additional_constraints = "", bool read_only = false,
        bool ignore_override = false) {
        m_watch_parameters[name] = cb;

        auto value = rclcpp::ParameterValue(default_value);

        auto descriptor = rcl_interfaces::msg::ParameterDescriptor();
        descriptor.name = name;
        descriptor.description = description;
        descriptor.additional_constraints = additional_constraints;
        descriptor.read_only = read_only;

        m_param_interface->declare_parameter(descriptor.name, value, descriptor,
                                             ignore_override);
    }

    /**
     * @brief Manually undeclare all parameters and detach callbacks.
     *
     * Safe to call multiple times.
     */
    void undeclare_parameters() { cleanup(); }

    /**
     * @brief Attach watcher to a node and register parameter update callback.
     *
     * @tparam NodeT rclcpp::Node or rclcpp_lifecycle::LifecycleNode
     * @param node Node instance to bind to.
     *
     * Must be called before declaring parameters when using default
     * constructor.
     */
    template <typename NodeT>
    void enable_watch_parameters(NodeT& node) {
        ensure_node(node);

        m_set_parameters_handle = node.add_on_set_parameters_callback(
            std::bind(&parameter_watcher::on_set_parameters_cb, this,
                      std::placeholders::_1));
    }

private:
    /**
     * @brief Ensure parameter interface is captured from the node.
     */
    template <typename NodeT>
    void ensure_node(NodeT& node) {
        if (!m_param_interface) {
            m_param_interface = node.get_node_parameters_interface();
        }
    }

    /**
     * @brief Cleanup resources.
     *
     * Removes callback and undeclares parameters owned by this watcher.
     */
    void cleanup();

    /**
     * @brief Internal ROS callback for parameter updates.
     *
     * Dispatches changes to registered user callbacks.
     *
     * @param parameters Updated parameters list.
     * @return SetParametersResult Success/failure flag.
     */
    SetParametersResult on_set_parameters_cb(
        const std::vector<rclcpp::Parameter>& parameters);

    /// Node parameters interface
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
        m_param_interface;

    /// Callback handle returned by ROS
    rclcpp::Node::OnSetParametersCallbackHandle::SharedPtr
        m_set_parameters_handle;

    /// Map: parameter name -> user callback
    std::unordered_map<std::string, ParameterFunctorT> m_watch_parameters;
};

}  // namespace clover2::common
