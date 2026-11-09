#pragma once

// clover2
#include <clover2_fcu/data/odometry.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

// STL
#include <memory>
#include <utility>

namespace clover2_fcu::capability {

/// @brief FCU odometry telemetry (the local-position estimate).
class odometry_interface : public telemetry_interface<data::odometry> {
public:
    using shared_ptr = std::shared_ptr<odometry_interface>;

    static constexpr const char* name = "odometry";

    explicit odometry_interface(rclcpp::Clock::SharedPtr clock)
        : telemetry_interface<data::odometry>(std::move(clock)) {}
};

}  // namespace clover2_fcu::capability
