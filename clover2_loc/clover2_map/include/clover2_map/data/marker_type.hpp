#pragma once

// STL
#include <cstdint>
#include <string>

namespace clover2_map {

struct marker_type {
    enum value : uint8_t {
        fixed = 0,    ///< pose in the map is known
        static_ = 1,  ///< not moving but pose unknown
        dynamic = 2,  ///< may moving
    };

    marker_type() = default;
    marker_type(value v);
    explicit marker_type(uint8_t raw);

    explicit operator uint8_t() const;

    std::string to_string() const;
    static marker_type from_string(const std::string& str);

    value v = value::fixed;
};

bool operator==(marker_type lhs, marker_type rhs);
bool operator!=(marker_type lhs, marker_type rhs);

}  // namespace clover2_map
