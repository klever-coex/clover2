#pragma once

// ROS2
#include <rclcpp/node_interfaces/node_base_interface.hpp>
#include <rclcpp/node_interfaces/node_clock_interface.hpp>
#include <rclcpp/node_interfaces/node_graph_interface.hpp>
#include <rclcpp/node_interfaces/node_interfaces.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_services_interface.hpp>
#include <rclcpp/node_interfaces/node_timers_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/rclcpp.hpp>

// third party
#include <yaml-cpp/yaml.h>

// STL
#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace clover2_fcu {

struct context {
    using node_interfaces = rclcpp::node_interfaces::NodeInterfaces<
        rclcpp::node_interfaces::NodeBaseInterface,
        rclcpp::node_interfaces::NodeClockInterface,
        rclcpp::node_interfaces::NodeGraphInterface,
        rclcpp::node_interfaces::NodeLoggingInterface,
        rclcpp::node_interfaces::NodeParametersInterface,
        rclcpp::node_interfaces::NodeServicesInterface,
        rclcpp::node_interfaces::NodeTimersInterface,
        rclcpp::node_interfaces::NodeTopicsInterface>;

    rclcpp::Logger logger{rclcpp::get_logger("clover2_fcu")};
    rclcpp::Clock::SharedPtr clock{std::make_shared<rclcpp::Clock>()};
    node_interfaces node{};
    rclcpp::CallbackGroup::SharedPtr group;
    std::string bridge_node;

    YAML::Node plugin_params{YAML::Load("{}")};

    YAML::Node params(const std::string& capability) const {
        return plugin_params[capability];
    }

    /// @brief Create a subscription on the fcu group.
    template <typename MessageT, typename CallbackT>
    rclcpp::Subscription<MessageT>::SharedPtr subscribe(
        const std::string& topic, const rclcpp::QoS& qos, CallbackT&& callback,
        rclcpp::SubscriptionOptions options = {}) const {
        options.callback_group = group;
        node_interfaces node_copy = node;

        return rclcpp::create_subscription<MessageT>(
            node_copy, topic, qos, std::forward<CallbackT>(callback), options);
    }

    /// @brief Create a publisher (membership of the fcu group is kept for
    ///        symmetry; publishing is thread-safe by itself).
    template <typename MessageT>
    rclcpp::Publisher<MessageT>::SharedPtr publisher(
        const std::string& topic, const rclcpp::QoS& qos) const {
        node_interfaces node_copy = node;
        return rclcpp::create_publisher<MessageT>(node_copy, topic, qos);
    }

    /// @brief Create a service client on the fcu group (or on an explicit
    ///        callback group for channels that must not contend with the
    ///        data-plane ordering - e.g. the blocking parameters channel).
    template <typename ServiceT>
    typename rclcpp::Client<ServiceT>::SharedPtr client(
        const std::string& service_name,
        const rclcpp::QoS& qos = rclcpp::ServicesQoS(),
        const rclcpp::CallbackGroup::SharedPtr& callback_group =
            nullptr) const {
        node_interfaces node_copy = node;
        return rclcpp::create_client<ServiceT>(
            node_copy.get_node_base_interface(),
            node_copy.get_node_graph_interface(),
            node_copy.get_node_services_interface(), service_name, qos,
            callback_group ? callback_group : group);
    }

    /// @brief Create a wall/ROS clock timer on the fcu group.
    rclcpp::TimerBase::SharedPtr timer(rclcpp::Duration period,
                                       std::function<void()> callback) const {
        node_interfaces node_copy = node;
        return rclcpp::create_timer(node_copy.get_node_base_interface(),
                                    node_copy.get_node_timers_interface(),
                                    clock, std::move(period),
                                    std::move(callback), group);
    }
};

}  // namespace clover2_fcu
