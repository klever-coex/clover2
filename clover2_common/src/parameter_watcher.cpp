#include <clover2/common/parameter_watcher.hpp>

namespace clover2::common {

void parameter_watcher::cleanup() {
    m_set_parameters_handle.reset();

    for (const auto& [name, callback] : m_watch_parameters) {
        if (m_param_interface->has_parameter(name)) {
            try {
                m_param_interface->undeclare_parameter(name);
            } catch (...) {
            }
        }
    }

    m_watch_parameters.clear();
}

parameter_watcher::SetParametersResult parameter_watcher::on_set_parameters_cb(
    const std::vector<rclcpp::Parameter>& parameters) {
    SetParametersResult result;
    result.successful = true;

    for (auto& p : parameters) {
        auto it = m_watch_parameters.find(p.get_name());
        if (it != m_watch_parameters.end()) {
            try {
                it->second(p);
            } catch (std::exception& ex) {
                result.successful = false;
                result.reason = ex.what();
                break;
            }
        }
    }

    return result;
}

}  // namespace clover2::common
