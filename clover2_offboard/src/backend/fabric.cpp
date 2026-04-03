#include <clover2_offboard/backend/fabric.hpp>
#include <clover2_offboard/backend/mavros.hpp>

namespace clover2_offboard::backend {

fabric::fabric() { add<mavros>(); }

fabric& fabric::instance() {
    static fabric inst;
    return inst;
}

std::shared_ptr<base_backend> fabric::create(const std::string& name,
                                             const context& ctx) const {
    auto it = m_builders.find(name);
    if (it != m_builders.end()) {
        return it->second(ctx);
    }
    return nullptr;
}

std::vector<std::string> fabric::list_backends() const {
    std::vector<std::string> names;
    names.reserve(m_builders.size());
    for (const auto& pair : m_builders) {
        names.push_back(pair.first);
    }
    return names;
}

}  // namespace clover2_offboard::backend
