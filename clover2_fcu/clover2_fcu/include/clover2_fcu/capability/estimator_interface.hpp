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

enum class estimator_source : uint8_t { vision = 0, mocap };

enum class estimator_reset : uint8_t {
    none = 0,
    height = 1 << 0,  ///< baro / ground-effect disturbances
    yaw = 1 << 1,     ///< magnetometer loss
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

    explicit estimator_interface(capability::base_interface::shared_ptr base)
        : telemetry_interface<data::estimator_status>(base) {}

    virtual result reset(estimator_reset scope = estimator_reset::full) = 0;

    virtual result set_origin(const data::geo_point& origin) = 0;

    virtual void publish_odometry(
        const data::odometry& odometry,
        estimator_source source = estimator_source::vision) = 0;
};

}  // namespace clover2_fcu::capability
