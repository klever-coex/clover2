#pragma once

// clover2
#include <clover2_fcu/data/flight_mode.hpp>

// STL
#include <functional>
#include <memory>
#include <optional>

namespace clover2_fcu::capability {

class commands_interface {
public:
    using shared_ptr = std::shared_ptr<commands_interface>;
    using result_callback = std::function<void(bool)>;

    static constexpr const char* name = "commands";

    virtual ~commands_interface() = default;

    /// @brief Arm the vehicle.
    virtual void arm(result_callback cb, bool run_preflight_checks = false) = 0;

    /// @brief Disarm the vehicle.
    virtual void disarm(result_callback cb) = 0;

    /// @brief Switch the flight mode.
    virtual void set_mode(result_callback cb, data::flight_mode mode) = 0;

    /// @brief Trigger an automatic takeoff (ack; the maneuver itself is
    ///        the navigation stack's action).
    virtual void takeoff(result_callback cb,
                         std::optional<double> altitude = std::nullopt,
                         std::optional<double> heading = std::nullopt) = 0;

    /// @brief Trigger an automatic landing (ack; the maneuver itself is
    ///        the navigation stack's action).
    virtual void land(result_callback cb) = 0;

    /// @brief Trigger a return-to-launch (ack; the maneuver itself is
    ///        the navigation stack's action).
    virtual void rtl(result_callback cb) = 0;
};

}  // namespace clover2_fcu::capability
