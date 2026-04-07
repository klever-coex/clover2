// clover2
#include <clover2/fcu_bridge/backend/mavros.hpp>
#include <rclcpp/logging.hpp>

// STL
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {

uint16_t type_mask(const std::optional<tf2::Vector3>& p,
                   const std::optional<tf2::Vector3>& v,
                   const std::optional<tf2::Vector3>& a,
                   const std::optional<double>& yaw,
                   const std::optional<double>& yaw_rate) {
    using PT = mavros_msgs::msg::PositionTarget;

    uint16_t ret = 0;

    if (!p.has_value()) {
        ret |= PT::IGNORE_PX | PT::IGNORE_PY | PT::IGNORE_PZ;
    }

    if (!v.has_value()) {
        ret |= PT::IGNORE_VX | PT::IGNORE_VY | PT::IGNORE_VZ;
    }

    if (!a.has_value()) {
        ret |= PT::IGNORE_AFX | PT::IGNORE_AFY | PT::IGNORE_AFZ;
    }

    if (!yaw.has_value()) {
        ret |= PT::IGNORE_YAW;
    }

    if (!yaw_rate.has_value()) {
        ret |= PT::IGNORE_YAW_RATE;
    }

    return ret;
}

clover2::fcu_bridge::data::mode mode_from_mavros(const std::string& mode_str) {
    using clover2::fcu_bridge::data::mode;
    using mavros_msgs::msg::State;

    const static std::unordered_map<std::string, mode::value> mavros_mode_map =
        {
            {State::MODE_PX4_MANUAL, mode::value::manual},
            {State::MODE_PX4_ACRO, mode::value::acro},
            {State::MODE_PX4_STABILIZED, mode::value::stabilize},
            {State::MODE_PX4_ALTITUDE, mode::value::altitude},
            {State::MODE_PX4_POSITION, mode::value::position},
            {State::MODE_PX4_OFFBOARD, mode::value::offboard},
            {State::MODE_PX4_TAKEOFF, mode::value::takeoff},
            {State::MODE_PX4_LAND, mode::value::land},
        };

    const auto state = mavros_mode_map.find(mode_str);

    if (state == mavros_mode_map.end()) {
        return mode::value::unknown;
    }

    return state->second;
}

std::string mode_to_mavros(const clover2::fcu_bridge::data::mode& m) {
    using clover2::fcu_bridge::data::mode;
    using mavros_msgs::msg::State;

    const static std::unordered_map<mode::value, std::string> mode_mavros_map =
        {
            {mode::value::manual, State::MODE_PX4_MANUAL},
            {mode::value::acro, State::MODE_PX4_ACRO},
            {mode::value::stabilize, State::MODE_PX4_STABILIZED},
            {mode::value::altitude, State::MODE_PX4_ALTITUDE},
            {mode::value::position, State::MODE_PX4_POSITION},
            {mode::value::offboard, State::MODE_PX4_OFFBOARD},
            {mode::value::takeoff, State::MODE_PX4_TAKEOFF},
            {mode::value::land, State::MODE_PX4_LAND},
        };

    const auto mavros_mode = mode_mavros_map.find(m);
    if (mavros_mode == mode_mavros_map.end()) {
        throw std::runtime_error("Unknown state");
    }

    return mavros_mode->second;
}

}  // namespace

namespace clover2::fcu_bridge::backend {

mavros::mavros(const context& ctx)
    : base_backend(ctx) {
    m_pos_setpoint_pub =
        rclcpp::create_publisher<mavros_msgs::msg::PositionTarget>(
            m_ctx, "/mavros/setpoint_raw/local", rclcpp::QoS(10));

    m_pose_sub = rclcpp::create_subscription<geometry_msgs::msg::PoseStamped>(
        m_ctx, "/mavros/local_position/pose", rclcpp::SensorDataQoS(),
        [&](const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
            m_pose = *msg;
            m_pose_received = true;
        });

    m_state_sub = rclcpp::create_subscription<mavros_msgs::msg::State>(
        m_ctx, "/mavros/state", rclcpp::QoS(10),
        [&](const mavros_msgs::msg::State::SharedPtr msg) {
            m_mavros_state = *msg;
            m_mode = mode_from_mavros(m_mavros_state.mode);
        });

    m_arming_client = rclcpp::create_client<mavros_msgs::srv::CommandBool>(
        m_ctx, "/mavros/cmd/arming");
    m_set_mode_client = rclcpp::create_client<mavros_msgs::srv::SetMode>(
        m_ctx, "/mavros/set_mode");
    m_land_client = rclcpp::create_client<mavros_msgs::srv::CommandTOL>(
        m_ctx, "/mavros/cmd/land");
}

bool mavros::ready() const {
    return m_pose_received && m_mavros_state.connected;
}

void mavros::arm() {
    if (!m_arming_client->service_is_ready()) {
        RCLCPP_WARN(get_logger(), "mavros/cmd/arming is not ready");
        return;
    }

    auto req = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
    req->value = true;
    m_arming_client->async_send_request(
        req, [this](rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture
                        future) {
            try {
                const auto res = future.get();
                if (!res->success) {
                    RCLCPP_WARN(get_logger(), "Arm rejected by MAVROS");
                }
            } catch (const std::exception& ex) {
                RCLCPP_WARN(get_logger(), "Arm request failed: %s", ex.what());
            }
        });
}

void mavros::disarm() {
    if (!m_arming_client->service_is_ready()) {
        RCLCPP_WARN(get_logger(), "mavros/cmd/arming is not ready");
        return;
    }

    auto req = std::make_shared<mavros_msgs::srv::CommandBool::Request>();
    req->value = false;
    m_arming_client->async_send_request(
        req, [this](rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedFuture
                        future) {
            try {
                const auto res = future.get();
                if (!res->success) {
                    RCLCPP_WARN(get_logger(), "Disarm rejected by MAVROS");
                }
            } catch (const std::exception& ex) {
                RCLCPP_WARN(get_logger(), "Disarm request failed: %s",
                            ex.what());
            }
        });
}

void mavros::land() {
    if (!m_land_client->service_is_ready()) {
        RCLCPP_WARN(get_logger(), "mavros/cmd/land is not ready");
        return;
    }

    auto req = std::make_shared<mavros_msgs::srv::CommandTOL::Request>();
    m_land_client->async_send_request(
        req,
        [this](
            rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedFuture future) {
            try {
                const auto res = future.get();
                if (!res->success) {
                    RCLCPP_WARN(get_logger(), "Land rejected by MAVROS");
                }
            } catch (const std::exception& ex) {
                RCLCPP_WARN(get_logger(), "Land request failed: %s", ex.what());
            }
        });
}

void mavros::set_mode(const data::mode& mode) {
    if (!m_set_mode_client->service_is_ready()) {
        RCLCPP_WARN(get_logger(), "mavros/set_mode is not ready");
        return;
    }

    auto req = std::make_shared<mavros_msgs::srv::SetMode::Request>();
    req->custom_mode = mode_to_mavros(mode);
    m_set_mode_client->async_send_request(
        req,
        [this](rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
            const auto res = future.get();

            try {
                const auto res = future.get();
                if (!res->mode_sent) {
                    const std::string msg =
                        "Switch to OFFBOARD was not accepted";
                    throw std::runtime_error(msg.c_str());
                }
            } catch (const std::exception& ex) {
                RCLCPP_WARN(get_logger(), "set_mode request failed: %s",
                            ex.what());
            }
        });
}

data::mode mavros::get_mode() const { return m_mode; }

void mavros::set_setpoint(const std::optional<tf2::Vector3> p,
                          const std::optional<tf2::Vector3> v,
                          const std::optional<tf2::Vector3> a,
                          const std::optional<double> yaw,
                          const std::optional<double> yaw_rate) {
    mavros_msgs::msg::PositionTarget msg;
    msg.header.stamp = get_clock()->now();
    msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
    msg.type_mask = type_mask(p, v, a, yaw, yaw_rate);

    auto p1 = p.value_or(tf2::Vector3(0.0, 0.0, 0.0));
    msg.position.x = p1.x();
    msg.position.y = p1.y();
    msg.position.z = p1.z();

    auto v1 = v.value_or(tf2::Vector3(0.0, 0.0, 0.0));
    msg.velocity.x = v1.x();
    msg.velocity.y = v1.y();
    msg.velocity.z = v1.z();

    auto a1 = a.value_or(tf2::Vector3(0.0, 0.0, 0.0));
    msg.acceleration_or_force.x = a1.x();
    msg.acceleration_or_force.y = a1.y();
    msg.acceleration_or_force.z = a1.z();

    msg.yaw = yaw.value_or(0.0);
    msg.yaw_rate = yaw_rate.value_or(0.0);

    m_pos_setpoint_pub->publish(msg);
}

}  // namespace clover2::fcu_bridge::backend
