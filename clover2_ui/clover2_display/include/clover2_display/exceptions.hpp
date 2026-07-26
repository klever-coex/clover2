#pragma once

#include <stdexcept>
#include <string>

namespace clover2_display {

// TODO: Replace this exception, for example: wrong size, wrong encoding, FPS
// limit.
class display_error : public std::runtime_error {
public:
    explicit display_error(const std::string& message)
        : std::runtime_error(message) {}
};

}  // namespace clover2_display
