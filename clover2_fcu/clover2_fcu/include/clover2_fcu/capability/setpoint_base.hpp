#pragma once

// clover2
#include <clover2_fcu/capability/base_interface.hpp>
#include <clover2_fcu_msgs/msg/mode_state.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// STL
#include <atomic>
#include <concepts>

namespace clover2_fcu {

struct mode_token {
    uint32_t mode_id{0};
};

namespace capability {

class setpoint_position_interface;
class setpoint_velocity_interface;
class setpoint_attitude_interface;
class setpoint_rates_interface;

class setpoint_base {
public:
    setpoint_base(base_interface::shared_ptr base, mode_token token);
    setpoint_base(const setpoint_base&) = delete;
    setpoint_base& operator=(const setpoint_base&) = delete;
    virtual ~setpoint_base() = default;

    bool stream_allowed() const noexcept {
        return m_active_mode_id.load(std::memory_order_acquire) ==
               m_mode_id;
    }

    void warn_muted() const {
        RCLCPP_WARN_THROTTLE(
            m_logger, *m_clock, 5000,
            "setpoint provider of the mode %u is muted: the mode does not "
            "hold the activation lease",
            m_mode_id);
    }

protected:
    base_interface::shared_ptr m_base;

private:
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
    uint32_t m_mode_id{0};
    std::atomic<uint32_t> m_active_mode_id{0};
    rclcpp::Subscription<clover2_fcu_msgs::msg::ModeState>::SharedPtr
        m_mode_state_sub;
};

template <typename T>
concept setpoint_contract = std::derived_from<T, setpoint_base> &&
                            (std::same_as<T, setpoint_position_interface> ||
                             std::same_as<T, setpoint_velocity_interface> ||
                             std::same_as<T, setpoint_attitude_interface> ||
                             std::same_as<T, setpoint_rates_interface>);

}  // namespace capability

}  // namespace clover2_fcu
