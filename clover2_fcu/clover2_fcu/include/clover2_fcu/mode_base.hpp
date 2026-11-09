#pragma once

// clover2
#include <clover2_fcu/capability/setpoint_position_interface.hpp>
#include <clover2_fcu/capability/setpoint_velocity_interface.hpp>
#include <clover2_fcu/capability/system_state_interface.hpp>
#include <clover2_fcu/client.hpp>
#include <clover2_fcu/context.hpp>
#include <clover2_fcu_msgs/msg/mode_state.hpp>
#include <clover2_fcu_msgs/srv/activate_mode.hpp>
#include <clover2_fcu_msgs/srv/register_mode.hpp>

// ROS2
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>

// third party
#include <bondcpp/bond.hpp>

// STL
#include <atomic>
#include <chrono>
#include <memory>
#include <string>

namespace clover2_fcu {

class mode_base {
public:
    using position_setpoint = capability::setpoint_position_interface;
    using velocity_setpoint = capability::setpoint_velocity_interface;

    struct settings {
        /// Mode name, unique among the modes registered with the bridge.
        std::string name;

        /// Setpoint update rate while the mode is active [Hz].
        float update_rate_hz{30.f};

        /// Blocking service wait timeout (registration).
        std::chrono::nanoseconds wait_timeout{std::chrono::seconds(5)};

        settings& with_name(std::string value) {
            name = std::move(value);
            return *this;
        }

        settings& with_update_rate(float value) {
            update_rate_hz = value;
            return *this;
        }

        settings& with_wait_timeout(std::chrono::nanoseconds value) {
            wait_timeout = value;
            return *this;
        }
    };

    /**
     * @brief Construct the mode and register it with the bridge.
     * @throws clover2_fcu::exception when registration fails.
     */
    mode_base(client::shared_ptr fcu, const settings& s);
    mode_base(const mode_base&) = delete;
    mode_base& operator=(const mode_base&) = delete;
    virtual ~mode_base();

    /// @brief Whether the activation lease is currently held.
    bool active() const { return m_active; }

    /// @brief Registered mode name.
    const std::string& name() const { return m_settings.name; }

    /// @brief Mode id assigned by the bridge.
    uint32_t mode_id() const { return m_mode_id; }

    /**
     * @brief Report the mode completed and release the activation lease
     *        (fire-and-forget ActivateMode with an empty name).
     */
    void completed();

protected:
    /// @brief Called when the bridge grants the activation lease.
    virtual void on_activate() {}

    /// @brief Called when the lease is revoked or the bond breaks.
    virtual void on_deactivate() {}

    /// @brief Called on the update timer while the mode is active.
    virtual void update_setpoint(double dt_s) { (void)dt_s; }

    /// @brief The flight stack interface of this mode.
    client::shared_ptr fcu() { return m_fcu; }

    /// @brief Setpoint providers (ordinary cached capabilities of the
    ///        client; streamed only from update_setpoint by convention).
    position_setpoint::shared_ptr position();
    velocity_setpoint::shared_ptr velocity();

    rclcpp::Logger get_logger() const { return m_logger; }

private:
    void setup_bond();

    void handle_mode_state(clover2_fcu_msgs::msg::ModeState::UniquePtr msg);

    /// @brief Bond callback — bondcpp runs it on the host node's default
    ///        group, outside the fcu group; relays into it.
    void handle_mode_bond_broken();

    /// @brief The relayed bond break, serialized with the mode_state
    ///        handling and the update timer.
    void handle_bond_broken();

    /// @brief The OFFBOARD entry sequence of an activated mode.
    void begin_activation();
    void handle_activation_state(const data::system_state& state);

    void start_update_timer();
    void stop_update_timer();
    void stop_streaming();

    client::shared_ptr m_fcu;
    rclcpp::Logger m_logger;
    const settings m_settings;

    /// The client-assembled environment (node interfaces, fcu group,
    /// bridge name, plugin params).
    context m_ctx;

    rclcpp::Client<clover2_fcu_msgs::srv::RegisterMode>::SharedPtr
        m_register_mode_client;
    rclcpp::Client<clover2_fcu_msgs::srv::ActivateMode>::SharedPtr
        m_activate_mode_client;

    std::shared_ptr<bond::Bond> m_bond;
    rclcpp::Subscription<clover2_fcu_msgs::msg::ModeState>::SharedPtr
        m_mode_state_sub;
    rclcpp::TimerBase::SharedPtr m_update_timer;
    rclcpp::Time m_last_update;

    /// The system_state subscription held while the mode waits for the
    /// OFFBOARD confirmation; released on the first OFFBOARD snapshot.
    capability::system_state_interface::token m_state_token;
    std::atomic<bool> m_set_mode_sent{false};

    std::atomic<bool> m_bond_broken{false};
    rclcpp::TimerBase::SharedPtr m_bond_relay_timer;

    uint32_t m_mode_id{0};
    std::atomic<bool> m_active{false};
};

}  // namespace clover2_fcu
