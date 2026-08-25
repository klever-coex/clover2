// clover2
#include <clover2_http_plugins/utils/detail/node_client.hpp>

// ROS2
#include <rclcpp/client.hpp>
#include <rclcpp/logging.hpp>
#include <rclcpp/subscription.hpp>

namespace clover2_http_plugins::utils::detail {

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

        // init current state
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
    info.lifecycle_state = m_state;

    return info;
}

void node_client::transition_cb(
    const lifecycle_msgs::msg::Transition::SharedPtr msg) {
    m_state = msg->label;
}

}  // namespace clover2_http_plugins::utils::detail
