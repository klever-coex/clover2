#include <clover2/cam_feature/base_plugin.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher.hpp>

#include <memory>

namespace clover2::cam_feature {

base_plugin::base_plugin()
    : m_logger(rclcpp::get_logger("base_plugin"))
    , m_clock(nullptr)
    , m_node_context(nullptr) {}

base_plugin::~base_plugin() {
    if (m_parameters_watcher) {
        m_parameters_watcher->undeclare_watcher_parameters();
    }
}

void base_plugin::configure(
    const std::string& name,
    const std::shared_ptr<clover2_common::node_context>& node_context,
    const std::shared_ptr<clover2::map::client>& map_client) {
    m_name = name;
    m_node_context = node_context;

    m_parameters_watcher = std::make_shared<
        clover2_common::node_interfaces::NodeParametersWatcher>(
        m_node_context->get_node_parameters_interface());

    m_logger =
        m_node_context->get_node_logging_interface()->get_logger().get_child(
            name);
    m_clock = m_node_context->get_node_clock_interface()->get_clock();

    on_configure(name, node_context, map_client);
    RCLCPP_DEBUG(get_logger(), "Configured");
}

void base_plugin::activate() { on_activate(); }

void base_plugin::deactivate() { on_deactivate(); }

void base_plugin::cleanup() {
    on_cleanup();
    m_node_context.reset();

    RCLCPP_DEBUG(get_logger(), "Cleaned up");
}

const std::string& base_plugin::get_name() const { return m_name; }

rclcpp::Logger base_plugin::get_logger() const { return m_logger; }

rclcpp::Clock::SharedPtr base_plugin::get_clock() const { return m_clock; }

}  // namespace clover2::cam_feature
