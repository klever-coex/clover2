// clover2
#include <clover2_fcu/backend/backend_base.hpp>

namespace clover2_fcu::backend {

std::vector<std::string> backend_base::capabilities() const {
    std::vector<std::string> names;
    names.reserve(m_builders.size() + m_setpoint_builders.size());

    for (const auto& [name, builder] : m_builders) {
        names.push_back(name);
    }
    for (const auto& [name, builder] : m_setpoint_builders) {
        names.push_back(name);
    }

    return names;
}

void backend_base::initialize() {
    if (m_loaded) {
        return;
    }

    do_initialize();
    m_loaded = true;
}

std::shared_ptr<void> backend_base::get(const std::string& capability) const {
    if (!m_loaded) {
        throw exception(
            "backend '{}' is not initialized (initialize() must be called "
            "before get())",
            protocol());
    }

    const auto builder = m_builders.find(capability);
    if (builder == m_builders.end()) {
        throw exception("backend '{}' does not serve capability '{}'",
                        protocol(), capability);
    }

    return builder->second();
}

std::shared_ptr<void> backend_base::create_setpoint(const std::string& setpoint,
                                                    mode_token token) const {
    if (!m_loaded) {
        throw exception(
            "backend '{}' is not initialized (initialize() must be called "
            "before create_setpoint())",
            protocol());
    }

    const auto builder = m_setpoint_builders.find(setpoint);
    if (builder == m_setpoint_builders.end()) {
        throw exception("backend '{}' does not serve setpoint '{}'", protocol(),
                        setpoint);
    }

    return builder->second(token);
}

}  // namespace clover2_fcu::backend
