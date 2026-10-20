#pragma once

namespace clover2_fcu::data {

struct geo_point {
    double lat{0.0};  ///< [rads]
    double lon{0.0};  ///< [rads]
    double alt{0.0};  ///< AMSL [m]
};

}  // namespace clover2_fcu::data
