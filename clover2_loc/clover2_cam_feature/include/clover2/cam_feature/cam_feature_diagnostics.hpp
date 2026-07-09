#pragma once

// clover2
#include <clover2_common/node_interfaces/node_diagnostics.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace clover2::cam_feature {

class CamFeatureDiagnostics
    : public clover2_common::node_interfaces::NodeDiagnostics {
public:
    RCLCPP_SMART_PTR_ALIASES_ONLY(CamFeatureDiagnostics)

    enum class error {
        missing_camera_info,
        invalid_map,
    };

    using callback =
        std::function<void(diagnostic_updater::DiagnosticStatusWrapper&)>;

    explicit CamFeatureDiagnostics(
        const std::shared_ptr<diagnostic_updater::Updater>& updater,
        const std::string& hardware_id);

    virtual ~CamFeatureDiagnostics();

    void set_error_callback(error error_code, callback callback);
    void remove_error_callback(error error_code);
    bool apply_error_callback(
        error error_code,
        diagnostic_updater::DiagnosticStatusWrapper& status) const;

private:
    RCLCPP_DISABLE_COPY(CamFeatureDiagnostics)

    std::unordered_map<error, callback> m_error_callbacks;
};

}  // namespace clover2::cam_feature
