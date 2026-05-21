#pragma once

#include <rclcpp/rclcpp.hpp>

namespace clover2_common {

class executor : public rclcpp::executors::MultiThreadedExecutor {
public:
    explicit executor(
        const rclcpp::ExecutorOptions& options = rclcpp::ExecutorOptions());

    ~executor() = default;

protected:
    void run(size_t thread_id);
};

}  // namespace clover2_common
