#include <clover2_common/node_runtime.hpp>

namespace clover2_common {

node_runtime::node_runtime(const std::string& node_name,
                           const rclcpp::NodeOptions& options)
    : m_node(std::make_shared<node>(node_name, options))
    , m_node_context(std::make_shared<node_context>(*m_node)) {
    m_executor.add_node(m_node);
}

node_runtime::~node_runtime() { stop(); }

void node_runtime::start() {
    if (m_started) {
        return;
    }

    m_started = true;
    m_spin_thread = std::thread([this]() { m_executor.spin(); });
}

void node_runtime::stop() {
    if (!m_started) {
        return;
    }

    m_executor.cancel();
    if (m_spin_thread.joinable()) {
        m_spin_thread.join();
    }
    m_started = false;
}

std::shared_ptr<node> node_runtime::get_node() const { return m_node; }

std::shared_ptr<node_context> node_runtime::get_node_context() const {
    return m_node_context;
}

}  // namespace clover2_common
