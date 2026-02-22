#pragma once

// ROS2
#include <rclcpp/rclpp.hpp>

// STL
#include <memory>

namespace clover2::offboard {

class client {
public:
    template <typename NodeT>
    explicit client(const NodeT& node, std::string& type) {
        m_node = node->create_sub_node("fcu_client");
    }

    virtual ~client();

private:
    void update_callback();

    rclcpp::Logger m_logger;
    rclcpp::Node::SharedPtr m_node;
    rclcpp::TimerBase::SharedPtr m_update_timer;
};

}  // namespace clover2::offboard
