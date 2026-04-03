#pragma once

// clover2
#include <clover2_offboard/backend/context.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

namespace clover2_offboard::backend {

class base_backend {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(base_backend)

    explicit base_backend(const context& ctx);
    virtual ~base_backend() = default;

    virtual void arm() = 0;
    virtual void disarm() = 0;

    virtual void enable_offboard() = 0;

    virtual void set_position_setpoint(double x, double y, double z, double yaw) = 0;

protected:
    rclcpp::Logger get_logger() const { return m_logger; }

    rclcpp::Clock::SharedPtr get_clock() const { return m_clock; };

    context m_ctx;

private:
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
};

}  // namespace clover2_offboard::backend
