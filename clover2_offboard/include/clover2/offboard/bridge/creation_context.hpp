#pragma once

#include <rclcpp/rclcpp.hpp>

namespace clover2::offboard::bridge {

struct creation_context {
    rclcpp::Node::SharedPtr node;
};

}
