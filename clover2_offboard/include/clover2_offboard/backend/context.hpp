#pragma once

// clover2
#include <clover2/common/node_context.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_offboard::backend {

struct context : public clover2::common::node_context {
    template <typename NodeT>
    explicit context(NodeT& node)
        : clover2::common::node_context(node) {}
};

}  // namespace clover2_offboard::backend
