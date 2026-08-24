#pragma once

// clover2
#include <clover2_common/data/stamped.hpp>

// STL
#include <cmath>

namespace clover2_fcu_bridge::data {

struct power {
    float voltage{NAN};
    float percentage{NAN};
};

using power_data = clover2_common::data::stamped<power>;

}  // namespace clover2_fcu_bridge::data
