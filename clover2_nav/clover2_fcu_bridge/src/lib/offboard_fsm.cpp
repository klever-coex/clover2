#include <clover2_fcu_bridge/data/mode.hpp>
#include <clover2_fcu_bridge/offboard_fsm.hpp>
#include <rclcpp/logging.hpp>
#include <tf2/LinearMath/Vector3.hpp>

#include <cmath>
#include <stdexcept>

namespace {

constexpr int64_t k_stream_before_offboard_ns = 250'000'000;
constexpr int64_t k_waiting_offboard_ns = 8'000'000'000;
constexpr int64_t k_waiting_arm_ns = 8'000'000'000;
constexpr int64_t k_stale_pose_ns = 500'000'000;

const char* state_name(clover2_fcu_bridge::offboard_fsm::state s) {
    using clover2_fcu_bridge::offboard_fsm;
    switch (s) {
        case offboard_fsm::state::idle:
            return "idle";
        case offboard_fsm::state::streaming_setpoints:
            return "streaming_setpoints";
        case offboard_fsm::state::waiting_offboard:
            return "waiting_offboard";
        case offboard_fsm::state::waiting_arm:
            return "waiting_arm";
        case offboard_fsm::state::hold:
            return "hold";
        case offboard_fsm::state::position:
            return "position";
        case offboard_fsm::state::navigation:
            return "navigation";
        case offboard_fsm::state::error:
            return "error";
    }
    return "?";
}

}  // namespace

namespace clover2_fcu_bridge {

offboard_fsm::offboard_fsm(rclcpp::Logger logger,
                           rclcpp::Clock::SharedPtr clock)
    : m_logger(std::move(logger))
    , m_clock(std::move(clock))
    , m_entered_at(m_clock->now()) {}

bool offboard_fsm::motion_state(state s) {
    switch (s) {
        case state::streaming_setpoints:
        case state::waiting_offboard:
        case state::waiting_arm:
        case state::hold:
        case state::position:
        case state::navigation:
            return true;
        case state::idle:
        case state::error:
            return false;
    }
    return false;
}

bool offboard_fsm::can_accept_command() const {
    return m_state == state::idle || m_state == state::hold;
}

bool offboard_fsm::quiescent_for_async() const {
    return m_state == state::idle || m_state == state::hold;
}

void offboard_fsm::reset() {
    m_state = state::idle;
    m_entered_at = m_clock->now();
    m_cmd_pose = geometry_msgs::msg::PoseStamped();
    m_hold_pose = geometry_msgs::msg::PoseStamped();
    m_safe_pose = geometry_msgs::msg::PoseStamped();
    m_last_current = geometry_msgs::msg::PoseStamped();
    m_cmd_is_navigation = false;
    m_cmd_speed = 0.3;
}

void offboard_fsm::request_navigate(
    const geometry_msgs::msg::PoseStamped& target, double speed,
    fcu_backend& fcu, navigation_controller& nav, rclcpp::Time now) {
    if (!can_accept_command()) {
        throw std::runtime_error("navigate from invalid state");
    }

    m_cmd_pose = target;
    m_cmd_is_navigation = true;
    m_cmd_speed = speed;

    handle_event(event::navigate_request, fcu, nav, now, "navigate");
}

void offboard_fsm::request_position(
    const geometry_msgs::msg::PoseStamped& target, fcu_backend& fcu,
    navigation_controller& nav, rclcpp::Time now) {
    if (!can_accept_command()) {
        throw std::runtime_error("set_position from invalid state");
    }

    m_cmd_pose = target;
    m_cmd_is_navigation = false;

    handle_event(event::position_request, fcu, nav, now, "set_position");
}

void offboard_fsm::transition(state to, rclcpp::Time now, fcu_backend& fcu,
                              navigation_controller& nav,
                              const std::string& reason) {
    if (m_state == to) {
        return;
    }

    if (to == state::error) {
        m_safe_pose = m_last_current;
        if (m_safe_pose.header.frame_id.empty()) {
            m_safe_pose = m_cmd_pose;
        }
    }

    RCLCPP_INFO(m_logger, "FSM %s -> %s (%s)", state_name(m_state),
                state_name(to), reason.c_str());

    m_state = to;
    m_entered_at = now;
    on_enter_state(fcu, nav, now);
}

void offboard_fsm::on_enter_state(fcu_backend& fcu, navigation_controller& nav,
                                  rclcpp::Time now) {
    (void)now;
    switch (m_state) {
        case state::idle:
            fcu.set_mode(data::mode{data::mode::value::position});
            break;
        case state::waiting_offboard:
            try {
                fcu.set_mode(data::mode{data::mode::value::offboard});
            } catch (const std::exception& ex) {
                RCLCPP_ERROR(m_logger, "set_mode(offboard) failed: %s",
                             ex.what());
                transition(state::error, m_clock->now(), fcu, nav,
                           "set_mode rejected");
            }
            break;
        case state::waiting_arm:
            fcu.arm();
            break;
        case state::navigation:
            nav.set_target(m_cmd_pose, m_cmd_speed);
            break;
        default:
            break;
    }
}

bool offboard_fsm::telemetry_ok(const geometry_msgs::msg::PoseStamped& pose,
                                rclcpp::Time now, std::string& detail) const {
    const auto& p = pose.pose.position;
    const auto& o = pose.pose.orientation;

    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) {
        detail = "pose position non-finite";
        return false;
    }

    if (!std::isfinite(o.x) || !std::isfinite(o.y) || !std::isfinite(o.z) ||
        !std::isfinite(o.w)) {
        detail = "pose orientation non-finite";
        return false;
    }

    const rclcpp::Time stamp(pose.header.stamp);
    if (stamp.nanoseconds() != 0) {
        const auto age = (now - stamp).nanoseconds();
        if (age < 0) {
            detail = "pose stamp in future";
            return false;
        }
        if (age > k_stale_pose_ns) {
            detail = "stale pose";
            return false;
        }
    }

    return true;
}

bool offboard_fsm::position_close_enough(
    const geometry_msgs::msg::PoseStamped& current,
    const navigation_controller& nav) const {
    tf2::Vector3 d{};
    double dyaw = 0.0;

    navigation_controller::pose_diff(current, m_cmd_pose, d, dyaw);

    constexpr double yaw_tol = 0.05;

    return d.length() < nav.tolerance() && std::abs(dyaw) < yaw_tol;
}

std::optional<offboard_fsm::event> offboard_fsm::poll_automatic_events(
    fcu_backend& fcu, navigation_controller& nav,
    const geometry_msgs::msg::PoseStamped& current_pose, rclcpp::Time now) {
    (void)nav;
    (void)current_pose;

    const int64_t elapsed = (now - m_entered_at).nanoseconds();

    if (motion_state(m_state) && !fcu.connected()) {
        return event::fcu_lost;
    }

    if (motion_state(m_state) && !fcu.ready()) {
        return event::fcu_lost;
    }

    if (m_state == state::streaming_setpoints) {
        if (elapsed >= k_stream_before_offboard_ns) {
            return event::stream_primed;
        }
    }

    if (m_state == state::waiting_offboard) {
        if (elapsed >= k_waiting_offboard_ns) {
            return event::timeout;
        }
        const auto mode_val = static_cast<data::mode::value>(fcu.get_mode());
        if (fcu.ready() && mode_val == data::mode::value::offboard) {
            return event::offboard_enabled;
        }
    }

    if (m_state == state::waiting_arm) {
        if (elapsed >= k_waiting_arm_ns) {
            return event::timeout;
        }
        if (fcu.ready() && fcu.is_armed()) {
            return event::armed;
        }
    }

    return std::nullopt;
}

void offboard_fsm::handle_event(event ev, fcu_backend& fcu,
                                navigation_controller& nav, rclcpp::Time now,
                                const std::string& detail) {
    const std::string reason = detail.empty() ? "event" : detail;

    switch (m_state) {
        case state::idle:
            if (ev == event::navigate_request ||
                ev == event::position_request) {
                transition(state::streaming_setpoints, now, fcu, nav, reason);
            }
            break;

        case state::hold:
            if (ev == event::navigate_request ||
                ev == event::position_request) {
                transition(state::streaming_setpoints, now, fcu, nav, reason);
            } else if (ev == event::backend_error) {
                transition(state::error, now, fcu, nav, reason);
            }
            break;

        case state::streaming_setpoints:
            if (ev == event::stream_primed) {
                transition(state::waiting_offboard, now, fcu, nav, reason);
            } else if (ev == event::fcu_lost || ev == event::timeout ||
                       ev == event::backend_error || ev == event::tf_failed) {
                transition(state::error, now, fcu, nav, reason);
            }
            break;

        case state::waiting_offboard:
            if (ev == event::offboard_enabled) {
                transition(state::waiting_arm, now, fcu, nav, reason);
            } else if (ev == event::timeout || ev == event::fcu_lost ||
                       ev == event::backend_error || ev == event::tf_failed) {
                transition(state::error, now, fcu, nav, reason);
            }
            break;

        case state::waiting_arm:
            if (ev == event::armed) {
                if (m_cmd_is_navigation) {
                    transition(state::navigation, now, fcu, nav, reason);
                } else {
                    transition(state::position, now, fcu, nav, reason);
                }
            } else if (ev == event::timeout || ev == event::fcu_lost ||
                       ev == event::backend_error || ev == event::tf_failed) {
                transition(state::error, now, fcu, nav, reason);
            }
            break;

        case state::navigation:
            if (ev == event::target_reached) {
                m_hold_pose = m_cmd_pose;
                transition(state::hold, now, fcu, nav, reason);
                nav.reset();
            } else if (ev == event::fcu_lost || ev == event::timeout ||
                       ev == event::backend_error || ev == event::tf_failed) {
                transition(state::error, now, fcu, nav, reason);
                nav.reset();
            }
            break;

        case state::position:
            if (ev == event::target_reached) {
                m_hold_pose = m_cmd_pose;
                transition(state::hold, now, fcu, nav, reason);
            } else if (ev == event::fcu_lost || ev == event::timeout ||
                       ev == event::backend_error || ev == event::tf_failed) {
                transition(state::error, now, fcu, nav, reason);
            }
            break;

        case state::error:
        default:
            break;
    }
}

void offboard_fsm::tick(fcu_backend& fcu, navigation_controller& nav,
                        const geometry_msgs::msg::PoseStamped& current_pose,
                        rclcpp::Time now,
                        geometry_msgs::msg::PoseStamped& out_setpoint) {
    m_last_current = current_pose;

    if (m_state == state::error) {
        out_setpoint = m_safe_pose;
        out_setpoint.header.stamp = now;
        return;
    }

    std::string detail;
    if (motion_state(m_state)) {
        if (!telemetry_ok(current_pose, now, detail)) {
            handle_event(event::backend_error, fcu, nav, now, detail);
        }
    }

    if (m_state != state::error) {
        if (const auto ev =
                poll_automatic_events(fcu, nav, current_pose, now)) {
            handle_event(*ev, fcu, nav, now, {});
        }
    }

    if (m_state == state::navigation) {
        nav.update(current_pose);
        if (nav.target_reached()) {
            handle_event(event::target_reached, fcu, nav, now, "nav_done");
        }
    } else if (m_state == state::position) {
        if (position_close_enough(current_pose, nav)) {
            handle_event(event::target_reached, fcu, nav, now, "pos_done");
        }
    }

    switch (m_state) {
        case state::idle:
            out_setpoint = current_pose;
            break;
        case state::streaming_setpoints:
        case state::waiting_offboard:
        case state::waiting_arm:
            out_setpoint = current_pose;
            break;
        case state::hold:
            out_setpoint = m_hold_pose;
            break;
        case state::position:
            out_setpoint = m_cmd_pose;
            break;
        case state::navigation:
            out_setpoint = nav.current_setpoint();
            break;
        case state::error:
            out_setpoint = m_safe_pose;
            break;
    }

    out_setpoint.header.stamp = now;
}

}  // namespace clover2_fcu_bridge
