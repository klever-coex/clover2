#pragma once

// clover2
#include <clover2_common/node_context.hpp>
#include <clover2_http_plugins/data/node_info.hpp>
#include <clover2_http_plugins/data/service_endpoint.hpp>
#include <clover2_http_plugins/data/topic_endpoint.hpp>

// ROS2
#include <rclcpp/client.hpp>
#include <rclcpp/subscription.hpp>

// msgs
#include <lifecycle_msgs/msg/transition.hpp>
#include <lifecycle_msgs/srv/get_state.hpp>

#include <string>
#include <string_view>

namespace clover2_http_plugins::utils::detail {

class node_client {
public:
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

private:
    void transition_cb(const lifecycle_msgs::msg::Transition::SharedPtr msg);

    std::vector<data::topic_endpoint> endpoints(bool publishers) const;
    std::vector<data::service_endpoint> service_endpoints(bool servers) const;

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
    std::string m_state;
    std::string m_full_name;

    rclcpp::Client<lifecycle_msgs::srv::GetState>::SharedPtr m_get_state_client;
    rclcpp::Subscription<lifecycle_msgs::msg::Transition>::SharedPtr
        m_transition_sub;
};

}  // namespace clover2_http_plugins::utils::detail
