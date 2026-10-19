/**
 * @file setpoint_base.hpp
 * @brief Provides the setpoint capability base, the mode gate and the
 *        setpoint contract concept.
 */

#pragma once

// clover2
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu_msgs/msg/mode_state.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <concepts>
#include <memory>

namespace clover2_fcu {

/**
 * @brief Mode binding of a setpoint provider.
 *
 * Issued by mode_base, which is the only place allowed to create setpoint
 * providers: the token is a mandatory constructor argument, so a provider
 * cannot exist outside a mode.
 */
struct mode_token {
    uint32_t mode_id{0};
};

namespace capability {

class setpoint_position_interface;
class setpoint_velocity_interface;
class setpoint_attitude_interface;
class setpoint_rates_interface;

/**
 * @brief Base class for setpoint capability contracts and providers.
 *
 * A setpoint provider is bound to a flight mode at construction (the
 * mandatory @ref mode_token) and streams only while that mode holds the
 * bridge activation lease — the counterpart of px4's "the FMU consumes
 * only the active mode's stream" arbitration.
 *
 * Backend providers must check stream_allowed() before every publish and
 * silently skip the publication when it is false (mute, not an exception);
 * @ref warn_muted logs a throttled hint when that happens.
 */
class setpoint_base {
public:
    setpoint_base(base_interface& base, mode_token token);
    setpoint_base(const setpoint_base&) = delete;
    setpoint_base& operator=(const setpoint_base&) = delete;
    virtual ~setpoint_base() = default;

    /// @brief Whether publishing is currently allowed for this provider:
    ///        the bound mode holds the bridge activation lease.
    bool stream_allowed() const noexcept {
        return m_active_mode_id == m_mode_id;
    }

    /// @brief Log a throttled warning when publishing is attempted while
    ///        the mode is not active. Call when stream_allowed() returned
    ///        false.
    void warn_muted() const {
        RCLCPP_WARN_THROTTLE(
            m_logger, *m_clock, 5000,
            "setpoint provider of the mode %u is muted: the mode does not "
            "hold the activation lease",
            m_mode_id);
    }

protected:
    base_interface& m_base;

private:
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
    uint32_t m_mode_id{0};
    uint32_t m_active_mode_id{0};
    rclcpp::Subscription<clover2_fcu_msgs::msg::ModeState>::SharedPtr
        m_mode_state_sub;
};

/**
 * @brief Supported setpoint contracts (backend_base::add_setpoint /
 *        client::create_setpoint).
 */
template <typename T>
concept setpoint_contract =
    std::derived_from<T, setpoint_base> &&
    (std::same_as<T, setpoint_position_interface> ||
     std::same_as<T, setpoint_velocity_interface> ||
     std::same_as<T, setpoint_attitude_interface> ||
     std::same_as<T, setpoint_rates_interface>);

}  // namespace capability

}  // namespace clover2_fcu
