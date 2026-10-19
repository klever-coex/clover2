/**
 * @file setpoint_rates_interface.hpp
 * @brief Provides the body rates setpoint capability contract.
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
 * @brief Body angular rates setpoint capability contract
 *        with collective thrust.
 *
 * Rates about body FLU axes (@ref clover2_fcu::body_frame),
 * thrust normalized to [0, 1].
 */
class setpoint_rates_interface : public setpoint_base {
public:
    using shared_ptr = std::shared_ptr<setpoint_rates_interface>;

    static constexpr const char* name = "setpoint_rates";

    explicit setpoint_rates_interface(base_interface& base, mode_token token)
        : setpoint_base(base, token) {}

    /**
     * @brief Rates setpoint update.
     * @param body_rates target rates about body FLU axes [rad/s].
     * @param thrust collective thrust, [0, 1].
     */
    virtual void update(const Eigen::Vector3d& body_rates, double thrust) = 0;
};

}  // namespace clover2_fcu::capability
