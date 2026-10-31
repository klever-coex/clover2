#pragma once

// clover2
#include <clover2_fcu/backend/backend_base.hpp>
#include <clover2_fcu/capability/all.hpp>
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu/capability/battery_interface.hpp>
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu/capability/odometry_interface.hpp>
#include <clover2_fcu/capability/parameters_interface.hpp>
#include <clover2_fcu/capability/setpoint_base.hpp>
#include <clover2_fcu/capability/system_state_interface.hpp>
#include <clover2_fcu_msgs/msg/fcu_config.hpp>
#include <clover2_fcu_msgs/srv/get_config.hpp>

// ROS2
#include <rclcpp/node.hpp>
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/rclcpp.hpp>

// third party
#include <pluginlib/class_loader.hpp>
#include <yaml-cpp/yaml.h>

// STL
#include <chrono>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_fcu {

class mode_base;

class client {
public:
    using shared_ptr = std::shared_ptr<client>;

    using required_interfaces = rclcpp::node_interfaces::NodeInterfaces<
        rclcpp::node_interfaces::NodeBaseInterface,
        rclcpp::node_interfaces::NodeTopicsInterface,
        rclcpp::node_interfaces::NodeServicesInterface,
        rclcpp::node_interfaces::NodeTimersInterface,
        rclcpp::node_interfaces::NodeGraphInterface,
        rclcpp::node_interfaces::NodeLoggingInterface>;

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

    explicit client(required_interfaces node_interfaces, settings s = {});

    client(const client&) = delete;
    client& operator=(const client&) = delete;
    ~client() = default;

    /// @brief The cached provider for the capability contract.
    template <capability::contract CapabilityT>
    std::shared_ptr<CapabilityT> get() {
        return std::static_pointer_cast<CapabilityT>(get(CapabilityT::name));
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

    /// @brief The client callback group (used by mode_base for its own
    ///        clients/subscriptions/timers).
    rclcpp::CallbackGroup::SharedPtr callback_group() const {
        return m_callback_group;
    }

private:
    /// @brief Create a setpoint provider bound to the mode.
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

    rclcpp::Logger get_logger() const { return m_logger; }

    friend class mode_base;

    settings m_settings;
    rclcpp::Logger m_logger;
    rclcpp::CallbackGroup::SharedPtr m_callback_group;
    rclcpp::Client<clover2_fcu_msgs::srv::GetConfig>::SharedPtr m_config_client;

    YAML::Node m_params;
    clover2_fcu_msgs::msg::FcuConfig m_config;

    /// @brief Guards the lazy backend creation (thread-safe client).
    std::mutex m_backend_mutex;

    pluginlib::ClassLoader<backend::backend_base> m_loader{
        "clover2_fcu", "clover2_fcu::backend::backend_base"};

    backend::backend_base::shared_ptr m_backend;
    mutable std::unordered_map<std::string, std::shared_ptr<void>> m_cache;
};

}  // namespace clover2_fcu
