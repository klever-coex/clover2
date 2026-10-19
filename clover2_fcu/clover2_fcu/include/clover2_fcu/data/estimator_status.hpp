/**
 * @file estimator_status.hpp
 * @brief Provides the estimator status snapshot.
 */

#pragma once

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_fcu::data {

/**
 * @brief Estimator health/fusion snapshot (v1 MVP set).
 *
 * Sufficient for a watchdog ("the estimator lost vision → request a
 * reset"); covariances and per-sensor innovations may follow.
 */
struct estimator_status {
    bool healthy{false};          ///< passed the innovation consistency tests
    bool local_valid{false};      ///< local position estimate converged
    bool global_valid{false};     ///< global (GPS-fused) estimate converged
    bool vision_fused{false};     ///< vision/mocap odometry is fused
    bool gps_fused{false};        ///< GPS is fused
    float innovation_ratio{0.f};  ///< worst innovation test ratio
    rclcpp::Time stamp{};         ///< estimator-side time of the snapshot
};

}  // namespace clover2_fcu::data
