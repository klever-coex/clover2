#pragma once

// clover2
#include <clover2_fcu/capability/battery_interface.hpp>
#include <clover2_fcu/context.hpp>

// ROS2
#include <rclcpp/subscription.hpp>

// msgs
#include <sensor_msgs/msg/battery_state.hpp>

// STL
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace clover2_fcu_mavros::capability {

using clover2_fcu::capability::battery_source;

class battery : public clover2_fcu::capability::battery_interface {
public:
    explicit battery(const clover2_fcu::context& ctx, std::string prefix);

    std::vector<uint8_t> ids() const override;

    battery_source::shared_ptr source(uint8_t id) override;

private:
    void handle_battery(const sensor_msgs::msg::BatteryState& msg);

    rclcpp::Subscription<sensor_msgs::msg::BatteryState>::SharedPtr m_sub;

    rclcpp::Clock::SharedPtr m_clock;
    mutable std::mutex m_mutex;
    std::map<uint8_t, battery_source::shared_ptr> m_sources;
};

}  // namespace clover2_fcu_mavros::capability
