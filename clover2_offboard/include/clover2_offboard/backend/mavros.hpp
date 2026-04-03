#pragma once

#include <clover2_offboard/backend/base_backend.hpp>
#include <clover2_offboard/backend/context.hpp>

#include <mavros_msgs/msg/position_target.hpp>
#include <mavros_msgs/srv/command_bool.hpp>
#include <mavros_msgs/srv/command_tol.hpp>
#include <mavros_msgs/srv/set_mode.hpp>
#include <rclcpp/rclcpp.hpp>

namespace clover2_offboard::backend {

class mavros : public base_backend {
public:
    static constexpr const char* name = "mavros";

    explicit mavros(const context& ctx);
    ~mavros() override = default;

    void arm() final;
    void disarm() final;

    void enable_offboard() final;

    void set_position_setpoint(double x, double y, double z, double yaw) final;

private:
    rclcpp::Publisher<mavros_msgs::msg::PositionTarget>::SharedPtr m_pos_setpoint_pub;

    rclcpp::Client<mavros_msgs::srv::CommandBool>::SharedPtr m_arming_client;
    rclcpp::Client<mavros_msgs::srv::SetMode>::SharedPtr m_set_mode_client;
    rclcpp::Client<mavros_msgs::srv::CommandTOL>::SharedPtr m_land_client;
};

}  // namespace clover2_offboard::backend
