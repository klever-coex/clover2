#pragma once

#include <clover2_fcu_bridge/fcu_backend.hpp>
#include <clover2_fcu_bridge/navigation_controller.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <rclcpp/clock.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/time.hpp>

#include <optional>
#include <string>

namespace clover2_fcu_bridge {

class offboard_fsm {
public:
    enum class state {
        idle,
        streaming_setpoints,
        waiting_offboard,
        waiting_arm,
        hold,
        position,
        navigation,
        error
    };

    enum class event {
        navigate_request,
        position_request,
        offboard_enabled,
        armed,
        target_reached,
        fcu_lost,
        tf_failed,
        timeout,
        backend_error,
        stream_primed
    };

    explicit offboard_fsm(rclcpp::Logger logger,
                          rclcpp::Clock::SharedPtr clock);

    state current_state() const { return m_state; }

    const geometry_msgs::msg::PoseStamped& command_pose() const {
        return m_cmd_pose;
    }

    bool can_accept_command() const;

    bool quiescent_for_async() const;

    void reset();

    void request_navigate(const geometry_msgs::msg::PoseStamped& target,
                          double speed, fcu_backend& fcu,
                          navigation_controller& nav, rclcpp::Time now);

    void request_position(const geometry_msgs::msg::PoseStamped& target,
                          fcu_backend& fcu, navigation_controller& nav,
                          rclcpp::Time now);

    void handle_event(event ev, fcu_backend& fcu, navigation_controller& nav,
                      rclcpp::Time now, const std::string& detail = {});

    void tick(fcu_backend& fcu, navigation_controller& nav,
              const geometry_msgs::msg::PoseStamped& current_pose,
              rclcpp::Time now, geometry_msgs::msg::PoseStamped& out_setpoint);

private:
    static bool motion_state(state s);

    void transition(state to, rclcpp::Time now, fcu_backend& fcu,
                    navigation_controller& nav, const std::string& reason);

    void on_enter_state(fcu_backend& fcu, navigation_controller& nav,
                        rclcpp::Time now);

    std::optional<event> poll_automatic_events(
        fcu_backend& fcu, navigation_controller& nav,
        const geometry_msgs::msg::PoseStamped& current_pose, rclcpp::Time now);

    bool telemetry_ok(const geometry_msgs::msg::PoseStamped& pose,
                      rclcpp::Time now, std::string& detail) const;

    bool position_close_enough(const geometry_msgs::msg::PoseStamped& current,
                               const navigation_controller& nav) const;

    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;

    state m_state{state::idle};
    rclcpp::Time m_entered_at{};
    geometry_msgs::msg::PoseStamped m_cmd_pose{};
    geometry_msgs::msg::PoseStamped m_hold_pose{};
    geometry_msgs::msg::PoseStamped m_safe_pose{};
    geometry_msgs::msg::PoseStamped m_last_current{};
    bool m_cmd_is_navigation{false};
    double m_cmd_speed{0.3};
};

}  // namespace clover2_fcu_bridge
