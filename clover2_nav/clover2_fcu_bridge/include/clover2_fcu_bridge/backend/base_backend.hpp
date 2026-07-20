#pragma once

// clover2
#include <clover2_fcu_bridge/backend/context.hpp>
#include <clover2_fcu_bridge/data/mode.hpp>

// ROS2
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/battery_state.hpp>
#include <sensor_msgs/msg/fluid_pressure.hpp>
#include <sensor_msgs/msg/imu.hpp>
#include <tf2/LinearMath/Vector3.hpp>

#include <cmath>
#include <cstdint>
#include <optional>
#include <string>

namespace clover2_fcu_bridge::backend {

template <typename T>
struct stamped_snapshot {
    bool received{false};
    rclcpp::Time stamp;
    T msg;
};

struct fcu_state_snapshot {
    bool received{false};
    bool connected{false};
    bool armed{false};
    std::string mode{"unknown"};
};

struct power_snapshot {
    bool received{false};
    float voltage{NAN};
    float percentage{NAN};
};

using imu_snapshot = stamped_snapshot<sensor_msgs::msg::Imu>;
using barometer_snapshot = stamped_snapshot<sensor_msgs::msg::FluidPressure>;

class base_backend {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(base_backend)

    explicit base_backend(const context& ctx);
    virtual ~base_backend() = default;

    virtual bool ready() const = 0;
    virtual bool connected() const { return true; }

    virtual bool is_armed() const = 0;
    virtual void arm() = 0;
    virtual void disarm() = 0;

    virtual void land() = 0;

    virtual void set_mode(const data::mode& mode) = 0;
    virtual data::mode get_mode() const = 0;

    virtual fcu_state_snapshot get_fcu_state_snapshot() const;
    virtual power_snapshot get_power_snapshot() const;
    virtual imu_snapshot get_imu_snapshot() const;
    virtual barometer_snapshot get_barometer_snapshot() const;

    virtual void set_setpoint(const std::optional<tf2::Vector3> p,
                              const std::optional<tf2::Vector3> v,
                              const std::optional<tf2::Vector3> a,
                              const std::optional<double> yaw,
                              const std::optional<double> yaw_rate) = 0;

    void set_position_setpoint(double x, double y, double z, double yaw);
    void set_velocity_setpoint(double vx, double vy, double vz,
                               double yaw_rate);

    const geometry_msgs::msg::PoseStamped& get_pose() const { return m_pose; }
    void enable_offboard() { set_mode(data::mode::value::offboard); }

protected:
    rclcpp::Logger get_logger() const { return m_logger; }
    rclcpp::Clock::SharedPtr get_clock() const { return m_clock; }

    context m_ctx;
    geometry_msgs::msg::PoseStamped m_pose;

private:
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
};

}  // namespace clover2_fcu_bridge::backend
