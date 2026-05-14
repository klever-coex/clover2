#include <clover2/cam_feature/base_plugin.hpp>

namespace clover2::cam_feature {

base_plugin::base_plugin()
    : m_logger(rclcpp::get_logger("base_plugin"))
    , m_clock(nullptr)
    , m_node_context(nullptr) {}

base_plugin::~base_plugin() = default;

}  // namespace clover2::cam_feature
