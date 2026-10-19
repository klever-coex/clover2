/**
 * @file setpoint_position_interface.hpp
 * @brief Provides the position setpoint capability contract.
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
 * @brief Smooth position setpoint capability contract (go-to style).
 *
 * Position in the canonical world ENU frame, heading from East
 * (ENU convention), see @ref clover2_fcu::world_frame. Unset optional
 * values are not controlled by the FCU.
 */
class setpoint_position_interface : public setpoint_base {
public:
    using shared_ptr = std::shared_ptr<setpoint_position_interface>;

    static constexpr const char* name = "setpoint_position";

    explicit setpoint_position_interface(base_interface& base, mode_token token)
        : setpoint_base(base, token) {}

    /**
     * @brief Position setpoint update.
     * @param position target position, world ENU [m].
     * @param heading target heading from East [rad], optional.
     */
    virtual void update(const Eigen::Vector3d& position,
                        std::optional<double> heading = {}) = 0;
};

}  // namespace clover2_fcu::capability
