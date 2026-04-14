#include <clover2/common/executor.hpp>

#include <pthread.h>

namespace clover2::common {

executor::executor(const rclcpp::ExecutorOptions& options)
    : rclcpp::executors::MultiThreadedExecutor(options) {}

void executor::run(size_t thread_id) {
    pthread_setname_np(pthread_self(), "test_exec");
    MultiThreadedExecutor::run(thread_id);
}

}  // namespace clover2::common
