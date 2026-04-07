#pragma once

// ROS2
#include <mavros_msgs/msg/state.hpp>

// STL
#include <string>
#include <unordered_map>

namespace clover2::fcu_bridge::data {

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
    mode(const value& mode)
        : m_value(mode) {}

    mode(const mode& other) = default;
    mode& operator=(const mode& other) = default;

    const std::string& to_str() const {
        using clover2::fcu_bridge::data::mode;

        const static std::unordered_map<mode::value, std::string> mode_str_map =
            {
                {mode::value::manual, "manual"},        //
                {mode::value::acro, "acro"},            //
                {mode::value::stabilize, "stabilize"},  //
                {mode::value::altitude, "altitude"},    //
                {mode::value::position, "position"},    //
                {mode::value::offboard, "offboard"},    //
                {mode::value::takeoff, "takeoff"},      //
                {mode::value::land, "land"},            //
                {mode::value::unknown, "unknown"},      //
            };

        const auto str = mode_str_map.find(m_value);

        if (str == mode_str_map.end()) {
            throw std::runtime_error("Unknown state");
        }

        return str->second;
    }

    operator std::string() const { return to_str(); }
    operator value() const { return m_value; }

private:
    value m_value{value::unknown};
};

}  // namespace clover2::fcu_bridge::data
