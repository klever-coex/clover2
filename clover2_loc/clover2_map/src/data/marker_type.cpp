#include <clover2_map/data/marker_type.hpp>

// STL
#include <stdexcept>

namespace clover2_map {

marker_type::marker_type(value v)
    : v(v) {}

marker_type::marker_type(uint8_t raw)
    : v(static_cast<value>(raw)) {}

marker_type::operator uint8_t() const { return static_cast<uint8_t>(v); }

std::string marker_type::to_string() const {
    switch (v) {
        case value::fixed:
            return "fixed";
        case value::static_:
            return "static";
        case value::dynamic:
            return "dynamic";
    }

    return "unknown";
}

marker_type marker_type::from_string(const std::string& str) {
    if (str == "fixed") {
        return marker_type(value::fixed);
    }
    if (str == "static") {
        return marker_type(value::static_);
    }
    if (str == "dynamic") {
        return marker_type(value::dynamic);
    }

    throw std::runtime_error("Unknown marker type: '" + str +
                             "' (supported: fixed, static, dynamic)");
}

bool operator==(marker_type lhs, marker_type rhs) { return lhs.v == rhs.v; }

bool operator!=(marker_type lhs, marker_type rhs) { return !(lhs == rhs); }

}  // namespace clover2_map
