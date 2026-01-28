#pragma once

// Eigen
#include <Eigen/Dense>

// STL
#include <array>

namespace clover2::aruco::util {

void cov_eigen_to_ros(const Eigen::Matrix<double, 6, 6>& e_cov,
                      std::array<double, 36>& cov) {
    Eigen::Map<Eigen::Matrix<double, 6, 6, Eigen::RowMajor>> map(cov.data());
    map = e_cov;
}

void cov_ros_to_eigen(const std::array<double, 36>& cov,
                      Eigen::Matrix<double, 6, 6> e_cov) {
    Eigen::Map<const Eigen::Matrix<double, 6, 6, Eigen::RowMajor>> map(
        cov.data());
    e_cov = map;
}

}  // namespace clover2::aruco::util
