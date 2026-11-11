#pragma once

// clover2
#include <clover2_fcu/data/flight_mode.hpp>

namespace clover2_fcu::data {

struct system_state {
    bool connected{false};
    bool armed{false};
    bool in_air{false};

    bool failsafe{false};

    flight_mode mode{};
};

}  // namespace clover2_fcu::data
