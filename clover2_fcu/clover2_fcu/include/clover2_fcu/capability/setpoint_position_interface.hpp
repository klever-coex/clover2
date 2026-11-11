#pragma once

// third party
#include <Eigen/Geometry>

// STL
#include <memory>
#include <optional>

namespace clover2_fcu::capability {

class setpoint_position_interface {
public:
    using shared_ptr = std::shared_ptr<setpoint_position_interface>;

    static constexpr const char* name = "setpoint_position";

    virtual ~setpoint_position_interface() = default;

    /// @brief Stream the next position setpoint; heading in radians.
    virtual void update(const Eigen::Vector3d& position,
                        std::optional<double> heading = {}) = 0;
};

}  // namespace clover2_fcu::capability
