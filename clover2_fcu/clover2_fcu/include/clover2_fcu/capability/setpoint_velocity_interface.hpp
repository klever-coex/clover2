/**
 * @file setpoint_velocity_interface.hpp
 * @brief Provides the velocity setpoint capability contract.
 */

#pragma once

// clover2
#include <clover2_fcu/capability/setpoint_base.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <memory>
#include <optional>

namespace clover2_fcu::capability {

/**
 * @brief Velocity setpoint capability contract.
 *
 * Linear velocity in the canonical world ENU frame
 * (@ref clover2_fcu::world_frame). Unset optional values are not
 * controlled by the FCU.
 */
class setpoint_velocity_interface : public setpoint_base {
public:
    using shared_ptr = std::shared_ptr<setpoint_velocity_interface>;

    static constexpr const char* name = "setpoint_velocity";

    explicit setpoint_velocity_interface(base_interface& base, mode_token token)
        : setpoint_base(base, token) {}

    /**
     * @brief Velocity setpoint update.
     * @param velocity target velocity, world ENU [m/s].
     * @param yaw_rate target yaw rate about body Z (up) [rad/s], optional.
     */
    virtual void update(const Eigen::Vector3d& velocity,
                        std::optional<double> yaw_rate = {}) = 0;
};

}  // namespace clover2_fcu::capability
