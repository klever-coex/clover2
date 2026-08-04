#include <clover2_ui/commands/diagnostics_runtime.hpp>

namespace clover2_ui::commands {

diagnostics_runtime::diagnostics_runtime(const std::string& topic)
    : m_node(std::make_shared<clover2_common::node>("clover2_diagnostics_tui"))
    , m_node_context(std::make_shared<clover2_common::node_context>(*m_node))
    , m_monitor(std::make_shared<api::diagnostics::diagnostic_monitor>(
          m_node_context, topic))
    , m_started(false) {
    m_executor.add_node(m_node);
}

diagnostics_runtime::~diagnostics_runtime() { stop(); }

void diagnostics_runtime::start() {
    if (m_started) {
        return;
    }

    m_started = true;
    m_spin_thread = std::thread([this]() { m_executor.spin(); });
}

void diagnostics_runtime::stop() {
    if (!m_started) {
        return;
    }

    m_executor.cancel();
    if (m_spin_thread.joinable()) {
        m_spin_thread.join();
    }
    m_started = false;
}

std::shared_ptr<api::diagnostics::diagnostic_monitor>
diagnostics_runtime::monitor() const {
    return m_monitor;
}

}  // namespace clover2_ui::commands
