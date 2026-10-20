#pragma once

// clover2
#include <clover2_fcu/capability/setpoint_base.hpp>

// third party
#include <Eigen/Geometry>

// STL
#include <memory>

namespace clover2_fcu::capability {

class setpoint_attitude_interface : public setpoint_base {
public:
    using shared_ptr = std::shared_ptr<setpoint_attitude_interface>;

    static constexpr const char* name = "setpoint_attitude";

    explicit setpoint_attitude_interface(base_interface::shared_ptr base, mode_token token)
        : setpoint_base(base, token) {}

    virtual void update(const Eigen::Quaterniond& attitude, double thrust) = 0;
};

}  // namespace clover2_fcu::capability
