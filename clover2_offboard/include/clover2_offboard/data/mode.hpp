#pragma once

// ROS2
#include <mavros_msgs/msg/state.hpp>

// STL
#include <string>

namespace clover2_offboard::data {

struct mode {
    enum value {
        manual,
        acro,
        stabilize,
        altitude,
        position,
        offboard,
        takeoff,
        land,

        unknown
    };

    mode() = default;
    mode(const value& mode);
    mode(const mavros_msgs::msg::State& msg);

    mode(const mode& other) = default;
    mode& operator=(const mode& other) = default;

    void from_mavros(const std::string& mode_str);

    const std::string& to_str() const;
    const std::string& to_mavros() const;

    operator std::string() const { return to_str(); }
    operator value() const { return m_value; }

private:
    value m_value{value::unknown};
};

}  // namespace clover2_offboard::data
