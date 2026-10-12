#pragma once

// clover2
#include <clover2/aruco/solver/base_solver.hpp>

namespace clover2::aruco::solver {

class average final : public base_solver {
public:
    static constexpr const char* name = "average";

    explicit average(base_solver::required_interfaces node_interfaces);

    std::optional<Eigen::Isometry3d> solve(
        const clover2_pose_msgs::msg::MarkerFrame& frame,
        const clover2_map::map& map) override final;

private:
    size_t m_min_markers{1};
};

}  // namespace clover2::aruco::solver
