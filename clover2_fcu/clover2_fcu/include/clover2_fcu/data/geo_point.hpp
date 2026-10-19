/**
 * @file geo_point.hpp
 * @brief Provides the geographic point type.
 */

#pragma once

namespace clover2_fcu::data {

/// @brief Geographic position: lat/lon [rad], alt AMSL [m].
struct geo_point {
    double lat{0.0};
    double lon{0.0};
    double alt{0.0};
};

}  // namespace clover2_fcu::data
