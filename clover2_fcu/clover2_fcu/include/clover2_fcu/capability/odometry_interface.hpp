#pragma once

// clover2
#include <clover2_fcu/data/odometry.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

namespace clover2_fcu::capability {

class odometry_interface : public telemetry_interface<data::odometry> {
public:
    using shared_ptr = std::shared_ptr<odometry_interface>;

    static constexpr const char* name = "odometry";

    explicit odometry_interface(base_interface::shared_ptr base)
        : telemetry_interface<data::odometry>(base) {}
};

}  // namespace clover2_fcu::capability
