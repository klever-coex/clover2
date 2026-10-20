#pragma once

// clover2
#include <clover2_fcu/capability/setpoint_base.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <memory>
#include <optional>

namespace clover2_fcu::capability {

class setpoint_velocity_interface : public setpoint_base {
public:
    using shared_ptr = std::shared_ptr<setpoint_velocity_interface>;

    static constexpr const char* name = "setpoint_velocity";

    explicit setpoint_velocity_interface(base_interface::shared_ptr base, mode_token token)
        : setpoint_base(base, token) {}

    virtual void update(const Eigen::Vector3d& velocity,
                        std::optional<double> yaw_rate = {}) = 0;
};

}  // namespace clover2_fcu::capability
