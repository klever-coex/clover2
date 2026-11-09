#pragma once

// clover2
#include <clover2_fcu/data/flight_mode.hpp>

namespace clover2_fcu::data {

struct system_state {
    /// Transport link to the FCU is up.
    bool connected{false};
    bool armed{false};
    bool in_air{false};

    /// FCU entered a failsafe state.
    bool failsafe{false};

    /// Current flight mode (native name; OFFBOARD for companion modes).
    flight_mode mode{};
};

}  // namespace clover2_fcu::data
