#pragma once

// clover2
#include <clover2_fcu/capability/setpoint_base.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <memory>

namespace clover2_fcu::capability {

class setpoint_rates_interface : public setpoint_base {
public:
    using shared_ptr = std::shared_ptr<setpoint_rates_interface>;

    static constexpr const char* name = "setpoint_rates";

    explicit setpoint_rates_interface(base_interface::shared_ptr base, mode_token token)
        : setpoint_base(base, token) {}

    virtual void update(const Eigen::Vector3d& body_rates, double thrust) = 0;
};

}  // namespace clover2_fcu::capability
