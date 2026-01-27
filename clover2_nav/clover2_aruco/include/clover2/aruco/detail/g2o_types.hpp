#pragma once

#include <g2o/core/base_unary_edge.h>
#include <g2o/core/base_vertex.h>

#include <Eigen/Geometry>

#include <algorithm>
#include <cmath>

namespace clover2::aruco::detail::g2o_types {

inline Eigen::Matrix3d hat3(const Eigen::Vector3d& v) {
    Eigen::Matrix3d m;
    m << 0.0, -v.z(), v.y(), v.z(), 0.0, -v.x(), -v.y(), v.x(), 0.0;
    return m;
}

inline Eigen::Matrix3d so3_exp(const Eigen::Vector3d& omega) {
    const double theta = omega.norm();
    const Eigen::Matrix3d Omega = hat3(omega);
    Eigen::Matrix3d R = Eigen::Matrix3d::Identity();

    if (theta < 1e-10) {
        R += Omega;
        return R;
    }

    const double sin_theta = std::sin(theta);
    const double cos_theta = std::cos(theta);
    R += (sin_theta / theta) * Omega;
    R += ((1.0 - cos_theta) / (theta * theta)) * (Omega * Omega);
    return R;
}

inline Eigen::Vector3d so3_log(const Eigen::Matrix3d& R) {
    const double cos_theta = (R.trace() - 1.0) * 0.5;
    const double cos_theta_clamped =
        std::min(1.0, std::max(-1.0, cos_theta));
    const double theta = std::acos(cos_theta_clamped);

    if (theta < 1e-10) {
        return Eigen::Vector3d::Zero();
    }

    Eigen::Vector3d omega;
    omega << R(2, 1) - R(1, 2), R(0, 2) - R(2, 0), R(1, 0) - R(0, 1);
    omega *= 0.5 / std::sin(theta);
    return omega * theta;
}

inline Eigen::Matrix<double, 6, 1> se3_log(const Eigen::Isometry3d& T) {
    const Eigen::Matrix3d R = T.rotation();
    const Eigen::Vector3d omega = so3_log(R);
    const double theta = omega.norm();
    const Eigen::Matrix3d Omega = hat3(omega);

    Eigen::Matrix3d V = Eigen::Matrix3d::Identity();
    if (theta < 1e-10) {
        V += 0.5 * Omega;
    } else {
        const double theta2 = theta * theta;
        const double A = std::sin(theta) / theta;
        const double B = (1.0 - std::cos(theta)) / theta2;
        const double C = (1.0 - A) / theta2;
        V += B * Omega + C * (Omega * Omega);
    }

    const Eigen::Vector3d rho = V.inverse() * T.translation();
    Eigen::Matrix<double, 6, 1> xi;
    xi.head<3>() = rho;
    xi.tail<3>() = omega;
    return xi;
}

inline Eigen::Isometry3d se3_exp(const Eigen::Matrix<double, 6, 1>& xi) {
    const Eigen::Vector3d rho = xi.head<3>();
    const Eigen::Vector3d omega = xi.tail<3>();
    const double theta = omega.norm();
    const Eigen::Matrix3d Omega = hat3(omega);

    Eigen::Matrix3d R = Eigen::Matrix3d::Identity();
    Eigen::Matrix3d V = Eigen::Matrix3d::Identity();

    if (theta < 1e-10) {
        R += Omega;
        V += 0.5 * Omega;
    } else {
        const double theta2 = theta * theta;
        const double sin_theta = std::sin(theta);
        const double cos_theta = std::cos(theta);
        R += (sin_theta / theta) * Omega;
        R += ((1.0 - cos_theta) / theta2) * (Omega * Omega);

        const double A = sin_theta / theta;
        const double B = (1.0 - cos_theta) / theta2;
        const double C = (1.0 - A) / theta2;
        V += B * Omega + C * (Omega * Omega);
    }

    Eigen::Isometry3d T = Eigen::Isometry3d::Identity();
    T.linear() = R;
    T.translation() = V * rho;
    return T;
}

class VertexPose : public g2o::BaseVertex<6, Eigen::Isometry3d> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    void setToOriginImpl() override { _estimate = Eigen::Isometry3d::Identity(); }

    void oplusImpl(const double* update) override {
        Eigen::Matrix<double, 6, 1> xi;
        for (int i = 0; i < 6; ++i) {
            xi[i] = update[i];
        }
        _estimate = se3_exp(xi) * _estimate;
    }

    bool read(std::istream& /* is */) override { return false; }
    bool write(std::ostream& /* os */) const override { return false; }
};

class EdgePoseMeasurement
    : public g2o::BaseUnaryEdge<6, Eigen::Isometry3d, VertexPose> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    void computeError() override {
        const auto* v = static_cast<const VertexPose*>(_vertices[0]);
        const Eigen::Isometry3d error_transform =
            _measurement.inverse() * v->estimate();
        _error = se3_log(error_transform);
    }

    bool read(std::istream& /* is */) override { return false; }
    bool write(std::ostream& /* os */) const override { return false; }
};

}  // namespace clover2::aruco::detail::g2o_types

