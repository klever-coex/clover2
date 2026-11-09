
#pragma once

// STL
#include <cstdint>

namespace clover2_fcu {

/// World/reference frame convention (ROS standard).
enum class world_frame : uint8_t { enu = 0 };
/// Body frame convention (ROS standard, REP-103).
enum class body_frame : uint8_t { flu = 0 };

}  // namespace clover2_fcu
