#pragma once

// clover2
#include <clover2/aruco/solver/base_solver.hpp>

// third party
#include <image_geometry/pinhole_camera_model.hpp>

namespace clover2::aruco::solver {

class global_pnp final : public base_solver {
public:
    static constexpr const char* name = "global_pnp";

    explicit global_pnp(base_solver::required_interfaces node_interfaces);

    std::optional<Eigen::Isometry3d> solve(
        const clover2_pose_msgs::msg::MarkerFrame& frame,
        const clover2_map::map& map) override final;

private:
    int m_ransac_iterations{100};
    double m_ransac_reproj_px{2.5};
    size_t m_min_markers{2};

    image_geometry::PinholeCameraModel m_camera_model;
};

}  // namespace clover2::aruco::solver
