#pragma once

// clover2
#include <clover2_common/data/stamped.hpp>
#include <clover2_fcu_bridge/data/mode.hpp>

namespace clover2_fcu_bridge::data {

struct fcu_state {
    bool connected{false};
    bool armed{false};
    mode flight_mode;
};

using fcu_state_data = clover2_common::data::stamped<fcu_state>;

}  // namespace clover2_fcu_bridge::data
