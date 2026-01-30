#pragma once

// ROS2
#include <rclcpp/rclpp.hpp>

// STL
#include <memory>

namespace clover2::offboard {

class client {
public:
    template <typename NodeT>
    explicit client(const NodeT& node, std::string& type)
        : m_base_interface(node->template get_node_base_interface())
        , m_logging_interface(node->template get_node_logging_interface())
        , m_parameters_interface(node->template get_node_parameters_interface())
        , m_logger(m_logging_interface->get_logger().get_child("client")) {}

    virtual ~client();

private:
    void update_callback();

    std::shared_ptr<rclcpp::node_interfaces::NodeBaseInterface>
        m_base_interface;
    std::shared_ptr<rclcpp::node_interfaces::NodeLoggingInterface>
        m_logging_interface;
    std::shared_ptr<rclcpp::node_interfaces::NodeParametersInterface>
        m_parameters_interface;

    rclcpp::Logger m_logger;
    rclcpp::TimerBase::SharedPtr m_update_timer;
};

}  // namespace clover2::offboard
