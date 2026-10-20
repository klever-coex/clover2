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

void backend_base::initialize(
    std::shared_ptr<capability::base_interface> base) {
    const std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (m_loaded) {
        return;
    }

    m_cache.emplace(capability::base_interface::name, base);
    do_initialize(base);
    m_loaded = true;
}

std::shared_ptr<void> backend_base::get(const std::string& capability) const {
    const std::lock_guard<std::recursive_mutex> lock(m_mutex);
    if (!m_loaded) {
        throw exception(
            "backend '{}' is not initialized (initialize() must be called "
            "before get())",
            protocol());
    }

    const auto cached = m_cache.find(capability);
    if (cached != m_cache.end()) {
        return cached->second;
    }

    const auto builder = m_builders.find(capability);
    if (builder == m_builders.end()) {
        throw exception("backend '{}' does not serve capability '{}'",
                        protocol(), capability);
    }

    // Runs under the recursive lock: the builder may call get() for its
    // dependencies; concurrent callers block until it finishes.
    auto provider = builder->second();
    m_cache.emplace(capability, provider);

    return provider;
}

std::shared_ptr<void> backend_base::create_setpoint(const std::string& setpoint,
                                                    mode_token token) const {
    const std::lock_guard<std::recursive_mutex> lock(m_mutex);
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

    // Not cached; the builder may call get() for its dependencies
    // (recursive lock).
    return builder->second(token);
}

}  // namespace clover2_fcu::backend
