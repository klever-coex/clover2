/**
 * @file client.hpp
 * @brief Provides the FCU interface client.
 */

#pragma once

// clover2
#include <clover2_fcu/backend/backend_base.hpp>
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu/capability/all.hpp>
#include <clover2_fcu/capability/battery_interface.hpp>
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu/capability/odometry_interface.hpp>
#include <clover2_fcu/capability/parameters_interface.hpp>
#include <clover2_fcu/capability/setpoint_base.hpp>
#include <clover2_fcu/capability/system_state_interface.hpp>
#include <clover2_fcu_msgs/msg/fcu_config.hpp>
#include <clover2_fcu_msgs/srv/get_config.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// third party
#include <pluginlib/class_loader.hpp>
#include <yaml-cpp/yaml.h>

// STL
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_fcu {

class mode_base;

/**
 * @brief Flight stack interface client.
 *
 * Embeds into any node and fetches the configuration from the @ref bridge
 * — the single configuration point (the constructor blocks until the
 * bridge answers or the wait timeout expires, @ref exception on failure).
 *
 * The client owns the @ref base_interface implementation and one backend
 * factory instance. Capability providers are cached singletons: the
 * factory creates a provider on the first get() and serves the same
 * instance afterwards, so the provider lives as long as the client and
 * telemetry subscriptions are released via the RAII token returned by
 * on_update():
 *
 *   auto odometry = fcu.get<odometry_interface>();  // cached handle
 *
 * Setpoints are the exception: they are bound to a flight mode and can be
 * created only inside mode_base (create_setpoint() is private, mode_base
 * is a friend; setpoint providers are not cached).
 *
 * Accessors throw @ref exception when the capability is not served by the
 * backend; check with @ref has (creates nothing) or declare requirements
 * via client::settings::require (validated at construction).
 */
class client {
public:
    /**
     * @brief Client construction settings (fluent).
     */
    class settings {
    public:
        settings() {}

        /// @brief Bridge node name (absolute), default "/fcu_bridge".
        settings& bridge_node(std::string node_name) {
            m_bridge_node = std::move(node_name);
            return *this;
        }

        /// @brief Overall wait timeout for the bridge connection.
        template <typename rep_t, typename period_t>
        settings& wait_timeout(std::chrono::duration<rep_t, period_t> timeout) {
            m_wait_timeout =
                std::chrono::duration_cast<std::chrono::nanoseconds>(timeout);
            return *this;
        }

        /// @brief Require a capability: construction fails when its
        ///        backend does not serve it.
        template <capability::contract T>
        settings& require() {
            m_required.push_back(T::name);
            return *this;
        }

        const std::string& bridge_node() const { return m_bridge_node; }
        std::chrono::nanoseconds wait_timeout() const { return m_wait_timeout; }
        const std::vector<std::string>& required() const { return m_required; }

    private:
        std::string m_bridge_node{"/fcu_bridge"};
        std::vector<std::string> m_required{};
        std::chrono::nanoseconds m_wait_timeout{std::chrono::seconds(10)};
    };

    /**
     * @brief Construct the client and connect it to the bridge.
     * @tparam NodeT Node type (clover2_common::node, lifecycle node, ...).
     * @param node the hosting node (must be spun while this runs).
     * @param s client settings.
     * @throws clover2_fcu::exception on connection or requirement failures.
     */
    template <typename NodeT>
    explicit client(NodeT& node, settings s = {})
        : m_settings(std::move(s))
        , m_base(std::make_shared<capability::base_interface_impl>(
              node, m_settings.bridge_node())) {
        m_callback_group = node.create_callback_group(
            rclcpp::CallbackGroupType::MutuallyExclusive);

        m_config_client =
            node.template create_client<clover2_fcu_msgs::srv::GetConfig>(
                m_settings.bridge_node() + "/get_config", rclcpp::ServicesQoS(),
                m_callback_group);

        connect();
    }

    client(const client&) = delete;
    client& operator=(const client&) = delete;
    ~client() = default;

    /**
     * @brief The cached provider for the capability contract.
     *
     * The same instance is returned on every call; the provider lives as
     * long as the client.
     * @throws clover2_fcu::exception when the capability is not served.
     */
    template <capability::contract CapabilityT>
    CapabilityT::shared_ptr get() {
        return backend()->template get<CapabilityT>();
    }

    /// @brief The cached provider for the capability name.
    std::shared_ptr<void> get(const std::string& capability);

    capability::system_state_interface::shared_ptr state() {
        return get<capability::system_state_interface>();
    }

    capability::odometry_interface::shared_ptr odometry() {
        return get<capability::odometry_interface>();
    }

    capability::battery_interface::shared_ptr battery() {
        return get<capability::battery_interface>();
    }

    capability::commands_interface::shared_ptr commands() {
        return get<capability::commands_interface>();
    }

    capability::parameters_interface::shared_ptr parameters() {
        return get<capability::parameters_interface>();
    }

    /// @brief Whether the capability is served by the backend.
    ///
    /// Reads the backend capability list; creates nothing.
    template <capability::contract T>
    bool has() {
        return has(T::name);
    }

    /// @brief Whether the capability is served by the backend.
    bool has(const std::string& capability);

    /// @brief The client runtime environment (providers, mode_base).
    capability::base_interface& base() { return *m_base; }

    /// @brief The client callback group (used by mode_base for its own
    ///        clients/subscriptions/timers).
    rclcpp::CallbackGroup::SharedPtr callback_group() const {
        return m_callback_group;
    }

private:
    /// @brief Create a setpoint provider bound to the mode.
    ///
    /// Only mode_base may call this: a setpoint provider cannot exist
    /// outside a mode, so none of the muted-outside-mode cases exist.
    template <capability::setpoint_contract SetpointT>
    SetpointT::shared_ptr create_setpoint(mode_token token) {
        return backend()->template create_setpoint<SetpointT>(token);
    }

    /// @brief Blocking bridge connection.
    /// @throws clover2_fcu::exception on failure.
    void connect();

    /// @brief Cache the configuration and validate requirements.
    void apply_config(const clover2_fcu_msgs::msg::FcuConfig& config);

    /// @brief Fail when a required capability is not served.
    void check_requirements();

    /// @brief The configured backend, initialized once.
    /// @throws clover2_fcu::exception on failure.
    backend::backend_base::shared_ptr backend();

    /// @brief Load (and cache) a backend by plugin name.
    /// @throws clover2_fcu::exception on failure.
    backend::backend_base::shared_ptr ensure_backend(
        const std::string& plugin_name);

    friend class mode_base;

    settings m_settings;
    std::shared_ptr<capability::base_interface_impl> m_base;

    rclcpp::CallbackGroup::SharedPtr m_callback_group;
    rclcpp::Client<clover2_fcu_msgs::srv::GetConfig>::SharedPtr m_config_client;

    YAML::Node m_params;
    clover2_fcu_msgs::msg::FcuConfig m_config;

    pluginlib::ClassLoader<backend::backend_base> m_loader{
        "clover2_fcu", "clover2_fcu::backend::backend_base"};
    std::unordered_map<std::string, backend::backend_base::shared_ptr>
        m_backends;
};

}  // namespace clover2_fcu
