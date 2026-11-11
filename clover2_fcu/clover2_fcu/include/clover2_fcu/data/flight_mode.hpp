#pragma once

// STL
#include <optional>
#include <string>
#include <string_view>

namespace clover2_fcu::data {

class flight_mode {
public:
    /// Well-known FCU modes.
    static const flight_mode& manual() noexcept {
        static const flight_mode mode{"manual"};
        return mode;
    }
    static const flight_mode& acrobatic() noexcept {
        static const flight_mode mode{"acrobatic"};
        return mode;
    }
    static const flight_mode& stabilize() noexcept {
        static const flight_mode mode{"stabilize"};
        return mode;
    }
    static const flight_mode& altitude_hold() noexcept {
        static const flight_mode mode{"altitude_hold"};
        return mode;
    }
    static const flight_mode& position_hold() noexcept {
        static const flight_mode mode{"position_hold"};
        return mode;
    }
    static const flight_mode& loiter() noexcept {
        static const flight_mode mode{"loiter"};
        return mode;
    }
    static const flight_mode& offboard() noexcept {
        static const flight_mode mode{"offboard"};
        return mode;
    }
    static const flight_mode& auto_mission() noexcept {
        static const flight_mode mode{"auto_mission"};
        return mode;
    }
    static const flight_mode& auto_takeoff() noexcept {
        static const flight_mode mode{"auto_takeoff"};
        return mode;
    }
    static const flight_mode& auto_land() noexcept {
        static const flight_mode mode{"auto_land"};
        return mode;
    }
    static const flight_mode& rtl() noexcept {
        static const flight_mode mode{"rtl"};
        return mode;
    }
    /// Reserved name for the no-mode / unparsed state.
    static const flight_mode& unknown() noexcept {
        static const flight_mode mode{"unknown"};
        return mode;
    }

    /// No mode (lease-free state).
    flight_mode() noexcept = default;

    /// Open slot: custom/native mode name.
    flight_mode(const char* name) noexcept
        : m_name(name) {}
    explicit flight_mode(std::string name) noexcept
        : m_name(std::move(name)) {}

    /// Mode name as reported by the FCU / bridge registry.
    const std::string& name() const noexcept { return m_name; }

    /// Alias of name() for logging.
    std::string_view to_string() const noexcept { return m_name; }

    /// Whether the mode is one of the well-known FCU modes.
    bool known() const noexcept;

    /// Parse a well-known mode name; arbitrary names are constructed
    /// directly, this only resolves the named accessors.
    static std::optional<flight_mode> from_string(const std::string& name);

    bool operator==(const flight_mode& other) const noexcept {
        return m_name == other.m_name;
    }

    bool operator==(std::string_view name) const noexcept {
        return m_name == name;
    }

private:
    std::string m_name{"unknown"};
};

}  // namespace clover2_fcu::data
