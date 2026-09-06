#pragma once

// clover2
#include <clover2_fcu_bridge/data/mode.hpp>

namespace clover2_fcu_bridge::data {

struct fcu_state {
    bool connected{false};
    bool armed{false};
    mode flight_mode;
};

}  // namespace clover2_fcu_bridge::data
