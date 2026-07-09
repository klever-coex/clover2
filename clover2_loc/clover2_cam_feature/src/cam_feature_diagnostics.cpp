#include <clover2/cam_feature/cam_feature_diagnostics.hpp>

namespace clover2::cam_feature {

CamFeatureDiagnostics::CamFeatureDiagnostics(
    const std::shared_ptr<diagnostic_updater::Updater>& updater,
    const std::string& hardware_id)
    : clover2_common::node_interfaces::NodeDiagnostics(updater, hardware_id) {}

CamFeatureDiagnostics::~CamFeatureDiagnostics() = default;

void CamFeatureDiagnostics::set_error_callback(error error_code,
                                               callback callback) {
    m_error_callbacks[error_code] = callback;
}

void CamFeatureDiagnostics::remove_error_callback(error error_code) {
    m_error_callbacks.erase(error_code);
}

bool CamFeatureDiagnostics::apply_error_callback(
    error error_code, diagnostic_updater::DiagnosticStatusWrapper& status)
    const {
    const auto callback_it = m_error_callbacks.find(error_code);
    if (callback_it == m_error_callbacks.end()) {
        return false;
    }

    callback_it->second(status);
    return true;
}

}  // namespace clover2::cam_feature
