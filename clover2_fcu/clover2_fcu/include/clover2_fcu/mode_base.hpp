#pragma once

// clover2
#include <clover2_fcu/capability/setpoint_attitude_interface.hpp>
#include <clover2_fcu/capability/setpoint_base.hpp>
#include <clover2_fcu/capability/setpoint_position_interface.hpp>
#include <clover2_fcu/capability/setpoint_rates_interface.hpp>
#include <clover2_fcu/capability/setpoint_velocity_interface.hpp>
#include <clover2_fcu/client.hpp>
#include <clover2_fcu/common/result.hpp>
#include <clover2_fcu_msgs/msg/mode_state.hpp>
#include <clover2_fcu_msgs/srv/activate_mode.hpp>
#include <clover2_fcu_msgs/srv/register_mode.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// third party
#include <bondcpp/bond.hpp>

// STL
#include <atomic>
#include <chrono>
#include <memory>
#include <string>

namespace clover2_fcu {

/**
 * @brief Base class for flight modes.
 *
 * A mode is the only legal context for streaming setpoints: it registers
 * with the bridge (RegisterMode) and receives a mode token; setpoint
 * providers created through the mode are bound to it by that token. The
 * bridge leases activation to one mode at a time; the lease is revoked
 * when the mode releases it (completed()) or when the mode bond breaks
 * (the mode node died).
 *
 * The virtual methods mirror px4_ros2::ModeBase: on_activate()/on_deactivate()
 * fire on lease transitions and update_setpoint(dt) is driven by a timer
 * that runs only while the mode is active.
 *
 * The hosting node must be spun while the mode registers (construction)
 * and while the mode is active (the update timer fires on the node's
 * executor). Do not call completed() with a synchronous assumption from
 * inside update_setpoint — it is fire-and-forget.
 */
class mode_base {
public:
    struct settings {
        /// Mode name, unique among the modes registered with the bridge.
        std::string name;

        /// Setpoint update rate while the mode is active [Hz].
        float update_rate_hz{30.f};

        /// Blocking service wait timeout (registration).
        std::chrono::nanoseconds wait_timeout{std::chrono::seconds(5)};

        /// @brief Fluent setter.
        settings& with_name(std::string value) {
            name = std::move(value);
            return *this;
        }

        /// @brief Fluent setter.
        settings& with_update_rate(float value) {
            update_rate_hz = value;
            return *this;
        }

        /// @brief Fluent setter.
        settings& with_wait_timeout(std::chrono::nanoseconds value) {
            wait_timeout = value;
            return *this;
        }
    };

    /**
     * @brief Construct the mode and register it with the bridge.
     * @throws clover2_fcu::exception when registration fails.
     */
    mode_base(client& fcu, const settings& s);
    mode_base(const mode_base&) = delete;
    mode_base& operator=(const mode_base&) = delete;
    virtual ~mode_base();

    bool active() const { return m_active; }

    /// @brief Registered mode name.
    const std::string& name() const { return m_settings.name; }

    /// @brief Mode id assigned by the bridge.
    uint32_t mode_id() const { return m_mode_id; }

    /**
     * @brief Report the mode completed and release the activation lease
     *        (fire-and-forget ActivateMode with an empty name).
     */
    void completed(result r);

protected:
    /// @brief Called when the bridge grants the activation lease.
    virtual void on_activate() {}

    /// @brief Called when the lease is revoked or the bond breaks.
    virtual void on_deactivate() {}

    /// @brief Called on the update timer while the mode is active.
    virtual void update_setpoint(double dt_s) { (void)dt_s; }

    /// @brief The flight stack interface of this mode.
    client& fcu() { return m_fcu; }

    /// @brief Setpoint provider handles (created lazily, owned by the mode;
    ///        streaming is gated by the mode lease).
    std::shared_ptr<capability::setpoint_position_interface> position();
    std::shared_ptr<capability::setpoint_velocity_interface> velocity();
    std::shared_ptr<capability::setpoint_attitude_interface> attitude();
    std::shared_ptr<capability::setpoint_rates_interface> rates();

private:
    void handle_mode_state(clover2_fcu_msgs::msg::ModeState::UniquePtr msg);
    void handle_mode_bond_broken();
    void start_update_timer();
    void stop_update_timer();
    void stop_streaming();

    client& m_fcu;
    const settings m_settings;

    rclcpp::CallbackGroup::SharedPtr m_callback_group;
    std::shared_ptr<rclcpp::Client<clover2_fcu_msgs::srv::RegisterMode>>
        m_register_mode_client;
    std::shared_ptr<rclcpp::Client<clover2_fcu_msgs::srv::ActivateMode>>
        m_activate_mode_client;

    std::shared_ptr<bond::Bond> m_bond;
    rclcpp::Subscription<clover2_fcu_msgs::msg::ModeState>::SharedPtr
        m_mode_state_sub;
    rclcpp::TimerBase::SharedPtr m_update_timer;
    rclcpp::Time m_last_update;

    uint32_t m_mode_id{0};
    std::atomic<bool> m_active{false};

    capability::setpoint_position_interface::shared_ptr m_position;
    capability::setpoint_velocity_interface::shared_ptr m_velocity;
    capability::setpoint_attitude_interface::shared_ptr m_attitude;
    capability::setpoint_rates_interface::shared_ptr m_rates;
};

}  // namespace clover2_fcu
