// clover2
#include <clover2_http/plugins/utils/detail/node_client.hpp>

// ROS2
#include <rclcpp/client.hpp>
#include <rclcpp/logging.hpp>
#include <rclcpp/subscription.hpp>

namespace clover2_http::plugins::utils::detail {

node_client::node_client(
    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr node_base,
    rclcpp::node_interfaces::NodeGraphInterface::SharedPtr node_graph,
    rclcpp::node_interfaces::NodeLoggingInterface::SharedPtr node_logging,
    rclcpp::node_interfaces::NodeTopicsInterface::SharedPtr node_topics,
    rclcpp::node_interfaces::NodeServicesInterface::SharedPtr node_services,
    rclcpp::node_interfaces::NodeParametersInterface::SharedPtr node_parameters,
    std::string_view name, std::string_view ns)
    : m_node_base(std::move(node_base))
    , m_node_graph(std::move(node_graph))
    , m_node_logging(std::move(node_logging))
    , m_node_topics(std::move(node_topics))
    , m_node_services(std::move(node_services))
    , m_node_parameters(std::move(node_parameters))
    , m_name(name)
    , m_ns(ns)
    , m_full_name(data::full_node_name(m_ns, m_name)) {
    const auto services = m_node_graph->get_service_names_and_types();

    m_lifecycle = services.contains(m_full_name + "/get_state");

    if (m_lifecycle) {
        m_transition_sub =
            rclcpp::create_subscription<lifecycle_msgs::msg::Transition>(
                m_node_parameters,                  //
                m_node_topics,                      //
                m_full_name + "/transition_event",  //
                rclcpp::QoS(1),                     //
                std::bind(&node_client::transition_cb, this,
                          std::placeholders::_1));

        m_get_state_client =
            rclcpp::create_client<lifecycle_msgs::srv::GetState>(
                m_node_base,      //
                m_node_graph,     //
                m_node_services,  //
                m_full_name + "/get_state");

        auto req = std::make_shared<lifecycle_msgs::srv::GetState::Request>();
        m_get_state_client->async_send_request(
            req,
            [this](rclcpp::Client<lifecycle_msgs::srv::GetState>::SharedFuture
                       future) {
                if (!future.valid()) {
                    RCLCPP_ERROR(
                        m_node_logging->get_logger().get_child("node_client"),
                        "Service call failed");
                    return;
                }

                auto resp = future.get();
                m_state = resp->current_state.label;
            });
    }
}

std::string_view node_client::full_name() const { return m_full_name; }

data::node_info node_client::get_node_info() const {
    data::node_info info;

    info.name = m_name;
    info.ns = m_ns;
    info.is_lifecycle = m_lifecycle;
    if (!m_state.empty()) {
        info.lifecycle_state = m_state;
    }

    return info;
}

std::vector<data::topic_endpoint> node_client::get_publishers() const {
    return endpoints(true);
}

std::vector<data::topic_endpoint> node_client::get_subscribes() const {
    return endpoints(false);
}

std::vector<data::service_endpoint> node_client::get_servers() const {
    return service_endpoints(true);
}

std::vector<data::service_endpoint> node_client::get_clients() const {
    return service_endpoints(false);
}

void node_client::transition_cb(
    const lifecycle_msgs::msg::Transition::SharedPtr msg) {
    m_state = msg->label;
}

std::vector<data::topic_endpoint> node_client::endpoints(
    bool publishers) const {
    const auto names_and_types =
        publishers
            ? m_node_graph->get_publisher_names_and_types_by_node(m_name, m_ns)
            : m_node_graph->get_subscriber_names_and_types_by_node(m_name,
                                                                   m_ns);

    std::vector<data::topic_endpoint> result;
    for (const auto& [topic, types] : names_and_types) {
        data::topic_endpoint endpoint;
        endpoint.info.name = topic;
        endpoint.info.type = types.empty() ? std::string{} : types.front();

        const auto infos =
            publishers ? m_node_graph->get_publishers_info_by_topic(topic)
                       : m_node_graph->get_subscriptions_info_by_topic(topic);

        for (const auto& info : infos) {
            if (data::full_node_name(info.node_namespace(), info.node_name()) ==
                m_full_name) {
                endpoint.info.type = info.topic_type();
                endpoint.qos_profile = data::to_qos(info.qos_profile());

                break;
            }
        }

        result.push_back(std::move(endpoint));
    }

    return result;
}

std::vector<data::service_endpoint> node_client::service_endpoints(
    bool servers) const {
    const auto names_and_types =
        servers
            ? m_node_graph->get_service_names_and_types_by_node(m_name, m_ns)
            : m_node_graph->get_client_names_and_types_by_node(m_name, m_ns);

    std::vector<data::service_endpoint> result;
    for (const auto& [service, types] : names_and_types) {
        data::service_endpoint endpoint;
        endpoint.info.name = service;
        endpoint.info.type = types.empty() ? std::string{} : types.front();

        result.push_back(std::move(endpoint));
    }

    return result;
}

}  // namespace clover2_http::plugins::utils::detail
