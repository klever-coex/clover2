#include <clover2_offboard/backend/mavros.hpp>
#include <rclcpp/create_client.hpp>
#include <rclcpp/create_publisher.hpp>
#include <rclcpp/create_service.hpp>

#include <rclcpp/node_interfaces/get_node_base_interface.hpp>
#include <rclcpp/node_interfaces/get_node_graph_interface.hpp>
#include <rclcpp/node_interfaces/get_node_services_interface.hpp>

#include <memory>

namespace clover2_offboard::backend {

namespace {

uint16_t positionYawTypeMask() {
    using PT = mavros_msgs::msg::PositionTarget;
    return PT::IGNORE_VX | PT::IGNORE_VY | PT::IGNORE_VZ | PT::IGNORE_AFX |
           PT::IGNORE_AFY | PT::IGNORE_AFZ | PT::IGNORE_YAW_RATE;
}

}  // namespace
mavros::mavros(const context& ctx)
    : base_backend(ctx) {
    m_pos_setpoint_pub =
        rclcpp::create_publisher<mavros_msgs::msg::PositionTarget>(
            m_ctx, "/mavros/setpoint_raw/local", rclcpp::QoS(10));

    m_arming_client = rclcpp::create_client<mavros_msgs::srv::CommandBool>(
        m_ctx, "/mavros/cmd/arming");
    m_set_mode_client = rclcpp::create_client<mavros_msgs::srv::SetMode>(
        m_ctx, "/mavros/set_mode");
    m_land_client = rclcpp::create_client<mavros_msgs::srv::CommandTOL>(
        m_ctx, "/mavros/cmd/land");
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

void mavros::enable_offboard() {
    if (!m_set_mode_client->service_is_ready()) {
        RCLCPP_WARN(get_logger(), "mavros/set_mode is not ready");
        return;
    }
    auto req = std::make_shared<mavros_msgs::srv::SetMode::Request>();
    req->custom_mode = "OFFBOARD";
    m_set_mode_client->async_send_request(
        req,
        [this](rclcpp::Client<mavros_msgs::srv::SetMode>::SharedFuture future) {
            try {
                const auto res = future.get();
                if (!res->mode_sent) {
                    RCLCPP_WARN(get_logger(),
                                "Switch to OFFBOARD was not accepted");
                }
            } catch (const std::exception& ex) {
                RCLCPP_WARN(get_logger(), "set_mode request failed: %s",
                            ex.what());
            }
        });
}

void mavros::set_position_setpoint(double x, double y, double z, double yaw) {
    mavros_msgs::msg::PositionTarget msg;
    msg.header.stamp = get_clock()->now();
    msg.coordinate_frame = mavros_msgs::msg::PositionTarget::FRAME_LOCAL_NED;
    msg.type_mask = positionYawTypeMask();
    msg.position.x = x;
    msg.position.y = y;
    msg.position.z = z;
    msg.yaw = yaw;
    m_pos_setpoint_pub->publish(msg);
}

}  // namespace clover2_offboard::backend
