#pragma once

// clover2
#include <clover2_fcu/common/result.hpp>
#include <clover2_fcu/data/estimator_status.hpp>
#include <clover2_fcu/data/geo_point.hpp>
#include <clover2_fcu/data/odometry.hpp>
#include <clover2_fcu/data/optical_flow.hpp>
#include <clover2_fcu/utils/sensor_publisher.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

// STL
#include <cstdint>
#include <memory>

namespace clover2_fcu::capability {

enum class estimator_source : uint8_t { vision = 0, mocap };

enum class estimator_reset : uint8_t {
    none = 0,
    height = 1 << 0,
    yaw = 1 << 1,
    position = 1 << 2,
    velocity = 1 << 3,
    full = 0xFF,
};

[[nodiscard]] constexpr estimator_reset operator|(
    estimator_reset lhs, estimator_reset rhs) noexcept {
    return static_cast<estimator_reset>(static_cast<uint8_t>(lhs) |
                                        static_cast<uint8_t>(rhs));
}

[[nodiscard]] constexpr estimator_reset operator&(
    estimator_reset lhs, estimator_reset rhs) noexcept {
    return static_cast<estimator_reset>(static_cast<uint8_t>(lhs) &
                                        static_cast<uint8_t>(rhs));
}

class estimator_interface : public telemetry_interface<data::estimator_status> {
public:
    using shared_ptr = std::shared_ptr<estimator_interface>;

    static constexpr const char* name = "estimator";

    explicit estimator_interface(rclcpp::Clock::SharedPtr clock)
        : telemetry_interface<data::estimator_status>(std::move(clock)) {}

    /// @brief Reset the estimator states selected by scope.
    virtual result reset(estimator_reset scope = estimator_reset::full) = 0;

    /// @brief Set the FCU global origin (the EKF GPS origin).
    virtual result set_origin(const data::geo_point& origin) = 0;

    /// @brief Odometry input handle; the source is baked into the closure.
    virtual utils::sensor_publisher<data::odometry> odometry_publisher(
        estimator_source source = estimator_source::vision) = 0;

    /// @brief Optical flow input handle; the sensor slot id is resolved by
    ///        the backend at creation (empty handle when no slot is left).
    virtual utils::sensor_publisher<data::optical_flow>
    optical_flow_publisher() = 0;
};

}  // namespace clover2_fcu::capability
