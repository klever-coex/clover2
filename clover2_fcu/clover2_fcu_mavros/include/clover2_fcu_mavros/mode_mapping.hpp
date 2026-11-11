#pragma once

// clover2
#include <clover2_fcu/data/flight_mode.hpp>

// STL
#include <array>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>

namespace clover2_fcu_mavros {

inline std::string mavros_mode_to_fcu(const std::string& mavros_mode) {
    static constexpr std::array<std::pair<std::string_view, std::string_view>,
                                12>
        k_table = {{
            {"MANUAL", "manual"},
            {"ACRO", "acrobatic"},
            {"STABILIZED", "stabilize"},
            {"ALTCTL", "altitude_hold"},
            {"POSCTL", "position_hold"},
            {"LOITER", "loiter"},
            {"AUTO.LOITER", "loiter"},
            {"AUTO.MISSION", "auto_mission"},
            {"AUTO.TAKEOFF", "auto_takeoff"},
            {"AUTO.LAND", "auto_land"},
            {"AUTO.RTL", "rtl"},
            {"OFFBOARD", "offboard"},
        }};

    for (const auto& [native, fcu] : k_table) {
        if (mavros_mode == native) {
            return std::string{fcu};
        }
    }

    std::string out = mavros_mode;
    for (auto& ch : out) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

inline std::optional<std::string> fcu_mode_to_mavros(
    const clover2_fcu::data::flight_mode& mode) {
    static constexpr std::array<std::pair<std::string_view, std::string_view>,
                                11>
        k_table = {{
            {"manual", "MANUAL"},
            {"acrobatic", "ACRO"},
            {"stabilize", "STABILIZED"},
            {"altitude_hold", "ALTCTL"},
            {"position_hold", "POSCTL"},
            {"loiter", "AUTO.LOITER"},
            {"auto_mission", "AUTO.MISSION"},
            {"auto_takeoff", "AUTO.TAKEOFF"},
            {"auto_land", "AUTO.LAND"},
            {"rtl", "AUTO.RTL"},
            {"offboard", "OFFBOARD"},
        }};

    for (const auto& [fcu, native] : k_table) {
        if (mode == fcu) {
            return std::string{native};
        }
    }

    return std::nullopt;
}

}  // namespace clover2_fcu_mavros
