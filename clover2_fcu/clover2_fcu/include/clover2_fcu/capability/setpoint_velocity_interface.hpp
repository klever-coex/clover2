#pragma once

// third party
#include <Eigen/Geometry>

// STL
#include <memory>
#include <optional>

namespace clover2_fcu::capability {

class setpoint_velocity_interface {
public:
    using shared_ptr = std::shared_ptr<setpoint_velocity_interface>;

    static constexpr const char* name = "setpoint_velocity";

    virtual ~setpoint_velocity_interface() = default;

    /// @brief Stream the next velocity setpoint; yaw_rate in rad/s.
    virtual void update(const Eigen::Vector3d& velocity,
                        std::optional<double> yaw_rate = {}) = 0;
};

}  // namespace clover2_fcu::capability
