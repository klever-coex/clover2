// clover2
#include <clover2/aruco/solver/average.hpp>
#include <clover2/aruco/solver/factory.hpp>
#include <clover2/aruco/solver/global_pnp.hpp>

// STL
#include <sstream>
#include <stdexcept>
#include <utility>

namespace clover2::aruco::solver {

factory::factory() {
    add<global_pnp>();
    add<average>();
}

factory& factory::instance() {
    static factory inst;
    return inst;
}

factory::value_type factory::create(
    const std::string& name,
    base_solver::required_interfaces node_interfaces) const {
    auto it = m_builders.find(name);

    if (it == m_builders.end()) {
        std::ostringstream oss;
        oss << "unknown solver '" << name << "'; available:";

        for (const auto& n : list_solvers()) {
            oss << " " << n;
        }

        throw std::runtime_error(oss.str());
    }

    return it->second(std::move(node_interfaces));
}

std::vector<std::string> factory::list_solvers() const {
    std::vector<std::string> names;
    names.reserve(m_builders.size());

    for (const auto& pair : m_builders) {
        names.push_back(pair.first);
    }

    return names;
}

}  // namespace clover2::aruco::solver
