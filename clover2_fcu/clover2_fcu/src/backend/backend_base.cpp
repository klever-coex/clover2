// clover2
#include <clover2_fcu/backend/backend_base.hpp>

namespace clover2_fcu::backend {

std::vector<std::string> backend_base::capabilities() const {
    std::vector<std::string> names;
    names.reserve(m_builders.size());

    for (const auto& [name, builder] : m_builders) {
        names.push_back(name);
    }

    return names;
}

void backend_base::initialize(const context& ctx) {
    if (m_loaded) {
        return;
    }

    m_context = ctx;
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

    return builder->second(m_context);
}

}  // namespace clover2_fcu::backend
