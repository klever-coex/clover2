// clover2
#include <clover2_common/node_interfaces/node_parameters_watcher.hpp>

#include <clover2/aruco/solver/base_solver.hpp>

namespace clover2::aruco::solver {

base_solver::base_solver(base_solver::required_interfaces node_interfaces)
    : m_parameters_watcher(
          std::make_shared<
              clover2_common::node_interfaces::NodeParametersWatcher>(
              node_interfaces.get_node_parameters_interface())) {}

base_solver::~base_solver() {
    if (m_parameters_watcher) {
        m_parameters_watcher->undeclare_watcher_parameters();
    }
}

}  // namespace clover2::aruco::solver
