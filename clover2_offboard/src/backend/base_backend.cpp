#include <clover2_offboard/backend/base_backend.hpp>

namespace clover2_offboard::backend {

base_backend::base_backend(const context& ctx)
    : m_ctx(ctx)
    , m_logger(m_ctx.get_node_logging_interface()->get_logger().get_child("backend"))
    , m_clock(m_ctx.get_node_clock_interface()->get_clock()) {}

}  // namespace clover2_offboard::backend
