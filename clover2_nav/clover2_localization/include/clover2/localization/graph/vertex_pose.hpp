#pragma once

#include <clover2/localization/graph/detail/se3_utils.hpp>

#include <g2o/core/base_vertex.h>

#include <Eigen/Geometry>

namespace clover2::localization::graph {

class vertex_pose : public g2o::BaseVertex<6, Eigen::Isometry3d> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    void setToOriginImpl() override {
        _estimate = Eigen::Isometry3d::Identity();
    }

    void oplusImpl(const double* update) override {
        Eigen::Matrix<double, 6, 1> xi;
        for (int i = 0; i < 6; ++i) {
            xi[i] = update[i];
        }
        _estimate = detail::se3_exp(xi) * _estimate;
    }

    bool read(std::istream& /* is */) override { return false; }
    bool write(std::ostream& /* os */) const override { return false; }
};

}  // namespace clover2::localization::graph
