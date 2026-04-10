#pragma once

#include <rclcpp/rclcpp.hpp>

namespace clover2::common {

class executor : public rclcpp::executors::MultiThreadedExecutor {
public:
    explicit executor(
        const rclcpp::ExecutorOptions& options = rclcpp::ExecutorOptions());

    ~executor() = default;
};

}  // namespace clover2::common
