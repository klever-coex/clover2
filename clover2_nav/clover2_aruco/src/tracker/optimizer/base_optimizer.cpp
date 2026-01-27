#include <clover2/aruco/optimizer/base_optimizer.hpp>
#include <rclcpp/node_interfaces/node_logging_interface.hpp>

namespace clover2::aruco::optimizer {

base_optimizer::base_optimizer(const clover2::aruco::optimizer::context& ctx)
    : m_ctx(ctx)
    , m_logger(ctx.node_logging->get_logger().get_child("optimizer")) {}

void base_optimizer::set_data_ready_callback(data_ready_callback_t callback) {
    m_data_ready_callback = std::move(callback);
}

void base_optimizer::notify_data_ready(const marker& pose,
                                       std::chrono::nanoseconds timestamp) {
    if (m_data_ready_callback) {
        m_data_ready_callback(pose, timestamp);
    }
}

}  // namespace clover2::aruco::optimizer
