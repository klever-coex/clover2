#pragma once

#include <clover2/pose/graph/detail/se3_utils.hpp>
#include <clover2/pose/graph/vertex_pose.hpp>

#include <g2o/core/base_binary_edge.h>

#include <Eigen/Geometry>

namespace clover2::pose::graph {

class edge_odom : public g2o::BaseBinaryEdge<6, Eigen::Isometry3d,
                                           vertex_pose, vertex_pose> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    void computeError() override {
        const auto* v0 = static_cast<const vertex_pose*>(_vertices[0]);
        const auto* v1 = static_cast<const vertex_pose*>(_vertices[1]);
        const Eigen::Isometry3d error_T =
            _measurement.inverse() * (v0->estimate().inverse() * v1->estimate());
        _error = detail::se3_log(error_T);
    }

    bool read(std::istream& /* is */) override { return false; }
    bool write(std::ostream& /* os */) const override { return false; }
};

}  // namespace clover2::pose::graph
