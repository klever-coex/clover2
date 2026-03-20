#pragma once

#include <g2o/core/base_vertex.h>

#include <Eigen/Core>

namespace clover2::localization::graph {

class vertex_point : public g2o::BaseVertex<3, Eigen::Vector3d> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    void setToOriginImpl() override { _estimate.setZero(); }

    void oplusImpl(const double* update) override {
        _estimate += Eigen::Vector3d(update[0], update[1], update[2]);
    }

    bool read(std::istream& /* is */) override { return false; }
    bool write(std::ostream& /* os */) const override { return false; }
};

}  // namespace clover2::localization::graph
