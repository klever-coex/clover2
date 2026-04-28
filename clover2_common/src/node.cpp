#include <clover2/common/node.hpp>
#include <rclcpp/node.hpp>

namespace clover2::common {

node::node(const std::string& node_name, const rclcpp::NodeOptions& options)
    : rclcpp::Node(node_name, options) {
    enable_diagnostic_updater();
    enable_parameter_watcher();
}

void node::enable_diagnostic_updater() {
    m_diagnostic_updater = std::make_shared<diagnostic_updater::Updater>(this);
    m_diagnostic_updater->setHardwareID(this->get_name());
}

void node::enable_parameter_watcher() {
    m_parameter_watcher = std::make_shared<parameter_watcher>(*this);
}

std::shared_ptr<diagnostic_updater::Updater> node::get_diagnostic_updater()
    const {
    return m_diagnostic_updater;
}

std::shared_ptr<parameter_watcher> node::get_parameter_watcher() const {
    return m_parameter_watcher;
}

}  // namespace clover2::common
