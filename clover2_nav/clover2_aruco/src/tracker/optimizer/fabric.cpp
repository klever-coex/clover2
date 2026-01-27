#include <clover2/aruco/optimizer/fabric.hpp>
#include <clover2/aruco/optimizer/simple_mean.hpp>
#include <clover2/aruco/optimizer/graph_linearizer.hpp>

namespace clover2::aruco::optimizer {

fabric::fabric() {
    add<simple_mean>();
    add<graph_linearizer>();
}

fabric& fabric::instance() {
    static fabric inst;
    return inst;
}

std::shared_ptr<base_optimizer> fabric::create(const std::string& name,
                                               const context& ctx) const {
    auto it = m_optimizers.find(name);
    if (it != m_optimizers.end()) {
        return it->second(ctx);
    }

    return nullptr;
}

std::vector<std::string> fabric::list_optimizers() const {
    std::vector<std::string> names;
    names.resize(m_optimizers.size());

    for (const auto& pair : m_optimizers) {
        names.push_back(pair.first);
    }

    return names;
}

}  // namespace clover2::aruco::optimizer
