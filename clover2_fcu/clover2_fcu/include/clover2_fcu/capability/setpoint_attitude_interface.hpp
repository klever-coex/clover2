/**
 * @file setpoint_attitude_interface.hpp
 * @brief Provides the attitude setpoint capability contract.
 */

#pragma once

// clover2
#include <clover2_fcu/capability/setpoint_base.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <memory>

namespace clover2_fcu::capability {

/**
 * @brief Attitude setpoint capability contract with collective thrust.
 *
 * Attitude quaternion from world(ENU) to body(FLU)
 * (@ref clover2_fcu::body_frame), thrust normalized to [0, 1].
 */
class setpoint_attitude_interface : public setpoint_base {
public:
    using shared_ptr = std::shared_ptr<setpoint_attitude_interface>;

    static constexpr const char* name = "setpoint_attitude";

    explicit setpoint_attitude_interface(base_interface& base, mode_token token)
        : setpoint_base(base, token) {}

    /**
     * @brief Attitude setpoint update.
     * @param attitude target attitude quaternion.
     * @param thrust collective thrust, [0, 1].
     */
    virtual void update(const Eigen::Quaterniond& attitude, double thrust) = 0;
};

}  // namespace clover2_fcu::capability
