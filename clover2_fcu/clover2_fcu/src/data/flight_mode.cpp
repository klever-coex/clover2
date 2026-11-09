// clover2
#include <clover2_fcu/data/flight_mode.hpp>

// STL
#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>

namespace clover2_fcu::data {

namespace {

constexpr std::array<std::string_view, 11> k_known_modes = {
    "manual",        "acrobatic", "stabilize", "altitude_hold",
    "position_hold", "loiter",    "offboard",  "auto_mission",
    "auto_takeoff",  "auto_land", "rtl",
};

}  // namespace

bool flight_mode::known() const noexcept {
    return std::find(k_known_modes.begin(), k_known_modes.end(), m_name) !=
           k_known_modes.end();
}

std::optional<flight_mode> flight_mode::from_string(const std::string& name) {
    // "unknown" is the reserved no-mode name, resolvable for parity.
    if (name == unknown().name()) {
        return unknown();
    }

    if (std::find(k_known_modes.begin(), k_known_modes.end(), name) ==
        k_known_modes.end()) {
        return std::nullopt;
    }

    return flight_mode{name};
}

}  // namespace clover2_fcu::data
