// clover2
#include <clover2/offboard/bridge/fabric.hpp>
#include <clover2/offboard/bridge/mavros_bridge.hpp>

namespace clover2::offboard::bridge {

fabric::fabric() {
    add<mavros_bridge>();
}

fabric& fabric::instance() {
    static fabric inst;
    return inst;
}

value_type fabric::create(const std::string& name,
                         const creation_context& ctx) const {
    auto it = m_bridges.find(name);
    if (it != m_bridges.end()) {
        return it->second(ctx);
    }

    return nullptr;
}

std::vector<std::string> fabric::list_bridges() const {
    std::vector<std::string> names;
    names.reserve(m_bridges.size());
    for (const auto& pair : m_bridges) {
        names.push_back(pair.first);
    }
    return names;
}

}  // namespace clover2::offboard::bridge
