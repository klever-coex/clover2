#include <clover2_offboard/data/mode.hpp>
#include <mavros_msgs/msg/state.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace {

using clover2_offboard::data::mode;
using mavros_msgs::msg::State;

const static std::unordered_map<std::string,
                                clover2_offboard::data::mode::value>
    mavros_mode_map = {
        {State::MODE_PX4_MANUAL, mode::value::manual},
        {State::MODE_PX4_ACRO, mode::value::acro},
        {State::MODE_PX4_STABILIZED, mode::value::stabilize},
        {State::MODE_PX4_ALTITUDE, mode::value::altitude},
        {State::MODE_PX4_POSITION, mode::value::position},
        {State::MODE_PX4_OFFBOARD, mode::value::offboard},
        {State::MODE_PX4_TAKEOFF, mode::value::takeoff},
        {State::MODE_PX4_LAND, mode::value::land},
};

const static std::unordered_map<clover2_offboard::data::mode::value,
                                std::string>
    mode_mavros_map = {
        {mode::value::manual, State::MODE_PX4_MANUAL},
        {mode::value::acro, State::MODE_PX4_ACRO},
        {mode::value::stabilize, State::MODE_PX4_STABILIZED},
        {mode::value::altitude, State::MODE_PX4_ALTITUDE},
        {mode::value::position, State::MODE_PX4_POSITION},
        {mode::value::offboard, State::MODE_PX4_OFFBOARD},
        {mode::value::takeoff, State::MODE_PX4_TAKEOFF},
        {mode::value::land, State::MODE_PX4_LAND},
};

const static std::unordered_map<clover2_offboard::data::mode::value,
                                std::string>
    mode_str_map = {
        {mode::value::manual, "manual"},
        {mode::value::acro, "acro"},
        {mode::value::stabilize, "stabilize"},
        {mode::value::altitude, "altitude"},
        {mode::value::position, "position"},
        {mode::value::offboard, "offboard"},
        {mode::value::takeoff, "takeoff"},
        {mode::value::land, "land"},
        {mode::value::unknown, "unknown"},
};

}  // namespace

namespace clover2_offboard::data {

mode::mode(const mavros_msgs::msg::State& msg) {  //
    from_mavros(msg.mode);
}

mode::mode(const value& mode)
    : m_value(mode) {}

void mode::from_mavros(const std::string& mode_str) {
    const auto state = mavros_mode_map.find(mode_str);

    if (state != mavros_mode_map.end()) {
        m_value = state->second;
    } else {
        m_value = value::unknown;
    }
}

const std::string& mode::to_str() const {
    const auto str = mode_str_map.find(m_value);

    if (str == mode_str_map.end()) {
        throw std::runtime_error("Unknown state");
    }

    return str->second;
}

const std::string& mode::to_mavros() const {
    const auto mavros_mode = mode_mavros_map.find(m_value);
    if (mavros_mode == mode_mavros_map.end()) {
        throw std::runtime_error("Unknown state");
    }

    return mavros_mode->second;
}

}  // namespace clover2_offboard::data
