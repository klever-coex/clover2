#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_http/plugins/data/lifecycle_state.hpp>
#include <clover2_http/plugins/data/lifecycle_transition.hpp>
#include <clover2_http/plugins/data/lifecycle_transition_description.hpp>
#include <clover2_http/plugins/data/node_info.hpp>
#include <clover2_http/plugins/data/service_endpoint.hpp>
#include <clover2_http/plugins/data/topic_endpoint.hpp>

// ROS2
#include <rclcpp/callback_group.hpp>
#include <rclcpp/client.hpp>
#include <rclcpp/subscription.hpp>

// msgs
#include <lifecycle_msgs/msg/transition_event.hpp>
#include <lifecycle_msgs/srv/change_state.hpp>
#include <lifecycle_msgs/srv/get_available_transitions.hpp>
#include <lifecycle_msgs/srv/get_state.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace clover2_http::plugins::utils::detail {

class node_client : public std::enable_shared_from_this<node_client> {
public:
    using transition_cb = std::function<void(bool, const std::string&)>;
    using available_transitions_cb = std::function<void(
        const std::vector<data::lifecycle_transition_description>&)>;

    explicit node_client(
        rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
        rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
        rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging,
        rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
        rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
        rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
            node_parameters,
        std::string_view name, std::string_view ns);

    template <typename NodeT>
    explicit node_client(const NodeT& node, std::string_view name,
                         std::string_view ns)
        : node_client(node->get_node_base_interface(),        //
                      node->get_node_graph_interface(),       //
                      node->get_node_logging_interface(),     //
                      node->get_node_topics_interface(),      //
                      node->get_node_services_interface(),    //
                      node->get_node_parameters_interface(),  //
                      std::move(name), std::move(ns)) {}

    std::string_view full_name() const;
    data::node_info get_node_info() const;

    std::vector<data::topic_endpoint> get_publishers() const;
    std::vector<data::topic_endpoint> get_subscribes() const;
    std::vector<data::service_endpoint> get_servers() const;
    std::vector<data::service_endpoint> get_clients() const;

    void transition(const data::lifecycle_transition& transition,
                    transition_cb&& cb);
    void get_available_transitions(available_transitions_cb&& cb);

private:
    void transition_callback(
        const lifecycle_msgs::msg::TransitionEvent::SharedPtr msg);

    std::vector<data::topic_endpoint> endpoints(bool publishers) const;
    std::vector<data::service_endpoint> service_endpoints(bool servers) const;

    rclcpp::Logger get_logger() const { return m_logger; }

    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr m_node_base;
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr m_node_graph;
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr m_node_logging;
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr m_node_topics;
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr m_node_services;
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr
        m_node_parameters;

    std::string m_name;
    std::string m_ns;
    bool m_lifecycle;
    std::string m_full_name;
    std::optional<data::lifecycle_state> m_state;

    rclcpp::Logger m_logger = rclcpp::get_logger("node_client");

    rclcpp::CallbackGroup::SharedPtr m_lifecycle_cb_group;

    rclcpp::Subscription<lifecycle_msgs::msg::TransitionEvent>::SharedPtr
        m_transition_sub;
    rclcpp::Client<lifecycle_msgs::srv::GetState>::SharedPtr m_get_state_client;
    rclcpp::Client<lifecycle_msgs::srv::GetAvailableTransitions>::SharedPtr
        m_get_available_transitions_client;
    rclcpp::Client<lifecycle_msgs::srv::ChangeState>::SharedPtr
        m_change_state_client;
};

}  // namespace clover2_http::plugins::utils::detail
