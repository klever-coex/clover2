#pragma once

#include <rclcpp/logger.hpp>

namespace clover2_fcu_bridge::fsm {

struct context {
    context() = default;

    context(const context&) = delete;
    context& operator=(const context&) = delete;

    context(context&&) = delete;
    context& operator=(context&&) = delete;

    rclcpp::Logger logger;
};

}  // namespace clover2_fcu_bridge::fsm
