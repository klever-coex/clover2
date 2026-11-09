#pragma once

// clover2
#include <clover2_fcu/capability/battery_interface.hpp>
#include <clover2_fcu/capability/commands_interface.hpp>
#include <clover2_fcu/capability/estimator_interface.hpp>
#include <clover2_fcu/capability/odometry_interface.hpp>
#include <clover2_fcu/capability/parameters_interface.hpp>
#include <clover2_fcu/capability/setpoint_position_interface.hpp>
#include <clover2_fcu/capability/setpoint_velocity_interface.hpp>
#include <clover2_fcu/capability/system_state_interface.hpp>

// STL
#include <string>
#include <vector>

namespace clover2_fcu::capability {

template <typename T>
concept contract = requires {
    typename T::shared_ptr;
    { T::name } -> std::convertible_to<const char*>;
};

using all = std::tuple<system_state_interface,       //
                       odometry_interface,           //
                       battery_interface,            //
                       commands_interface,           //
                       parameters_interface,         //
                       estimator_interface,          //
                       setpoint_position_interface,  //
                       setpoint_velocity_interface>;

namespace detail {

constexpr bool names_equal(const char* a, const char* b);

template <typename... Ts>
consteval bool names_unique() {
    const char* names[] = {Ts::name...};

    for (std::size_t i = 0; i < sizeof...(Ts); ++i) {
        for (std::size_t j = i + 1; j < sizeof...(Ts); ++j) {
            if (names_equal(names[i], names[j])) {
                return false;
            }
        }
    }

    return true;
}

}  // namespace detail

const std::vector<std::string>& all_names();

}  // namespace clover2_fcu::capability
