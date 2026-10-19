/**
 * @file odometry_interface.hpp
 * @brief Provides the odometry capability contract.
 */

#pragma once

// clover2
#include <clover2_fcu/data/odometry.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

namespace clover2_fcu::capability {

/**
 * @brief Odometry telemetry capability contract.
 *
 * Served by a backend provider (FCU transport or an external ROS 2 source,
 * depending on the bridge configuration). Consumers needing strictly
 * time-aligned state use last() to get a consistent snapshot.
 */
class odometry_interface : public telemetry_interface<data::odometry> {
public:
    using shared_ptr = std::shared_ptr<odometry_interface>;

    static constexpr const char* name = "odometry";

    explicit odometry_interface(base_interface& base)
        : telemetry_interface<data::odometry>(base) {}
};

}  // namespace clover2_fcu::capability
