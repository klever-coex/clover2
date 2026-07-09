#include <clover2_common/node_interfaces/node_diagnostics.hpp>

namespace clover2_common::node_interfaces {

class NodeDiagnostics::NodeDiagnosticsImpl {
public:
    NodeDiagnosticsImpl(
        const std::shared_ptr<diagnostic_updater::Updater>& updater,
        const std::string& hardware_id)
        : m_updater(updater) {
        m_updater->setHardwareID(hardware_id);
    }

    void add(const std::string& name, DiagnosticTaskCallbackT callback) {
        m_updater->add(name, callback);
    }

    void remove_by_name(const std::string& name) {
        m_updater->removeByName(name);
    }

    void force_update() { m_updater->force_update(); }

    std::shared_ptr<diagnostic_updater::Updater> get_updater() {
        return m_updater;
    }

private:
    std::shared_ptr<diagnostic_updater::Updater> m_updater;
};

NodeDiagnostics::NodeDiagnostics(
    const std::shared_ptr<diagnostic_updater::Updater>& updater,
    const std::string& hardware_id)
    : m_impl(new NodeDiagnosticsImpl(updater, hardware_id)) {}

NodeDiagnostics::~NodeDiagnostics() {}

void NodeDiagnostics::add(const std::string& name,
                          DiagnosticTaskCallbackT callback) {
    m_impl->add(name, callback);
}

void NodeDiagnostics::remove_by_name(const std::string& name) {
    m_impl->remove_by_name(name);
}

void NodeDiagnostics::force_update() { m_impl->force_update(); }

std::shared_ptr<diagnostic_updater::Updater> NodeDiagnostics::get_updater() {
    return m_impl->get_updater();
}

}  // namespace clover2_common::node_interfaces
