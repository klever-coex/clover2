#pragma once

#include <diagnostic_msgs/msg/diagnostic_array.hpp>
#include <diagnostic_msgs/msg/diagnostic_status.hpp>
#include <rclcpp/node_interfaces/node_parameters_interface.hpp>
#include <rclcpp/node_interfaces/node_topics_interface.hpp>
#include <rclcpp/rclcpp.hpp>

#include <functional>
#include <memory>
#include <string>

namespace clover2_common::diagnostics {

class client {
public:
    using message_type = diagnostic_msgs::msg::DiagnosticArray;
    using status_type = diagnostic_msgs::msg::DiagnosticStatus;
    using callback_type = std::function<void(const message_type&)>;
    using node_parameters_interface =
        rclcpp::node_interfaces::NodeParametersInterface;
    using node_topics_interface = rclcpp::node_interfaces::NodeTopicsInterface;

    static constexpr const char* default_topic = "/diagnostics_agg";

    client(node_parameters_interface::SharedPtr node_parameters,
           node_topics_interface::SharedPtr node_topics,
           const std::string& topic = default_topic);

    template <typename NodeT>
    explicit client(const NodeT& node, const std::string& topic = default_topic)
        : client(node->get_node_parameters_interface(),
                 node->get_node_topics_interface(), topic) {}

    ~client() = default;

    void set_callback(callback_type callback);
    void cleanup();
    static bool status_changed(const status_type& previous,
                               const status_type& current);

private:
    void diagnostics_callback(message_type::ConstSharedPtr msg);

    callback_type m_callback;
    rclcpp::Subscription<diagnostic_msgs::msg::DiagnosticArray>::SharedPtr
        m_sub;
};

}  // namespace clover2_common::diagnostics
