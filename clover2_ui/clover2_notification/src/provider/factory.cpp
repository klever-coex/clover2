#include <clover2_notification/provider/diagnostics.hpp>
#include <clover2_notification/provider/factory.hpp>

namespace clover2_notification::provider {

factory::factory() {
    add<diagnostics>();
}

factory& factory::instance() {
    static factory inst;
    return inst;
}

factory::value_type factory::create(const std::string& name) const {
    const auto it = m_builders.find(name);
    if (it != m_builders.end()) {
        return it->second();
    }
    return nullptr;
}

std::vector<std::string> factory::list_providers() const {
    std::vector<std::string> names;
    names.reserve(m_builders.size());
    for (const auto& [name, _] : m_builders) {
        names.push_back(name);
    }
    return names;
}

}  // namespace clover2_notification::provider
