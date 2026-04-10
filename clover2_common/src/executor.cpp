#include <clover2/common/executor.hpp>

namespace clover2::common {

executor::executor(const rclcpp::ExecutorOptions& options)
    : rclcpp::executors::MultiThreadedExecutor(options) {}

}  // namespace clover2::common
