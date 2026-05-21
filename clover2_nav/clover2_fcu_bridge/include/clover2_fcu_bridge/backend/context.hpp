#pragma once

// clover2
#include <clover2_common/node_context.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_fcu_bridge::backend {

struct context : public clover2_common::node_context {
    template <typename NodeT>
    explicit context(NodeT& node)
        : clover2_common::node_context(node) {}
};

}  // namespace clover2_fcu_bridge::backend
