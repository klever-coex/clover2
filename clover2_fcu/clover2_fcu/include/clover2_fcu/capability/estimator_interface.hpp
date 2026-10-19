/**
 * @file estimator_interface.hpp
 * @brief Provides the estimator capability contract.
 */

#pragma once

// clover2
#include <clover2_fcu/common/result.hpp>
#include <clover2_fcu/data/estimator_status.hpp>
#include <clover2_fcu/data/geo_point.hpp>
#include <clover2_fcu/data/odometry.hpp>
#include <clover2_fcu/utils/telemetry_interface.hpp>

// STL
#include <cstdint>
#include <memory>

namespace clover2_fcu::capability {

/// @brief Source of the computer-side odometry fed into the estimator.
enum class estimator_source : uint8_t { vision = 0, mocap };

/// @brief Reset scope of the estimator (bitmask).
enum class estimator_reset : uint8_t {
    none = 0,
    height = 1 << 0,   ///< baro / ground-effect disturbances
    yaw = 1 << 1,      ///< magnetometer loss
    position = 1 << 2,
    velocity = 1 << 3,
    full = 0xFF,
};

/// @brief Combine reset scopes.
[[nodiscard]] constexpr estimator_reset operator|(estimator_reset lhs,
                                                  estimator_reset rhs) noexcept {
    return static_cast<estimator_reset>(static_cast<uint8_t>(lhs) |
                                        static_cast<uint8_t>(rhs));
}

/// @brief Intersection of two reset scopes.
[[nodiscard]] constexpr estimator_reset operator&(estimator_reset lhs,
                                                  estimator_reset rhs) noexcept {
    return static_cast<estimator_reset>(static_cast<uint8_t>(lhs) &
                                        static_cast<uint8_t>(rhs));
}

/**
 * @brief Estimator capability contract: EKF control, computer-side
 *        odometry feed and status telemetry.
 *
 * Transport-agnostic: the same API serves a flight-controller EKF
 * (through the backend transport, e.g. mavros or uXRCE-DDS) and a ROS 2
 * estimator node (e.g. with a betaflight flight controller). The backend
 * converts the canonical frames (world ENU, body FLU) to the estimator
 * frames; per-capability configuration comes from
 * base_interface::params(name).
 *
 * Methods follow the layer conventions: reset()/set_origin() are
 * synchronous request/acknowledge calls reporting through @ref result
 * (result::unsupported is a legitimate answer when the transport lacks
 * the command); publish_odometry() is fire-and-forget for high-rate
 * streaming — the channel health is monitored via last_valid().
 */
class estimator_interface : public telemetry_interface<data::estimator_status> {
public:
    using shared_ptr = std::shared_ptr<estimator_interface>;

    static constexpr const char* name = "estimator";

    explicit estimator_interface(capability::base_interface& base)
        : telemetry_interface<data::estimator_status>(base) {}

    /**
     * @brief Reset the estimator, or the requested state subset.
     * @param scope states to reset; @ref estimator_reset::full by default.
     * @returns result of the operation.
     */
    virtual result reset(estimator_reset scope = estimator_reset::full) = 0;

    /**
     * @brief Set the global origin of the estimator (datum).
     * @param origin origin position, lat/lon [rad], alt AMSL [m].
     * @returns result of the operation.
     */
    virtual result set_origin(const data::geo_point& origin) = 0;

    /**
     * @brief Feed a computer-side odometry snapshot into the estimator.
     * @param odometry odometry snapshot, world ENU / body FLU.
     * @param source odometry source (estimators may use distinct inputs).
     */
    virtual void publish_odometry(
        const data::odometry& odometry,
        estimator_source source = estimator_source::vision) = 0;
};

}  // namespace clover2_fcu::capability
