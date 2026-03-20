#pragma once

#include <clover2/localization/graph/vertex_point.hpp>
#include <clover2/localization/graph/vertex_pose.hpp>

#include <g2o/core/base_binary_edge.h>

#include <Eigen/Core>

namespace clover2::localization::graph {

class edge_projection
    : public g2o::BaseBinaryEdge<2, Eigen::Vector2d, vertex_pose, vertex_point> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    void set_camera_matrix(const Eigen::Matrix3d& K) { m_K = K; }

    void computeError() override {
        const auto* v_pose = static_cast<const vertex_pose*>(_vertices[0]);
        const auto* v_point = static_cast<const vertex_point*>(_vertices[1]);

        const Eigen::Vector3d p_cam =
            v_pose->estimate().inverse() * v_point->estimate();
        if (p_cam.z() <= 0.0) {
            _error = Eigen::Vector2d::Constant(1e6);
            return;
        }

        const Eigen::Vector3d p_proj = m_K * p_cam;
        _error = Eigen::Vector2d(p_proj.x() / p_proj.z(), p_proj.y() / p_proj.z()) -
                _measurement;
    }

    bool read(std::istream& /* is */) override { return false; }
    bool write(std::ostream& /* os */) const override { return false; }

private:
    Eigen::Matrix3d m_K = Eigen::Matrix3d::Identity();
};

}  // namespace clover2::localization::graph
