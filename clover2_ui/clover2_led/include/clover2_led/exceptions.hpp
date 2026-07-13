#pragma once

#include <stdexcept>
#include <string>

namespace clover2_led {

class exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

namespace data {

class frequency_to_high : public clover2_led::exception {
public:
    frequency_to_high()
        : clover2_led::exception("Data rate is too high") {}
};

}  // namespace data

namespace device {

class frame_size_mismatch : public clover2_led::exception {
public:
    frame_size_mismatch(size_t expected, size_t actual)
        : clover2_led::exception(
              "frame size mismatch: expected " + std::to_string(expected) +
              ", got " + std::to_string(actual)) {}
};

}  // namespace device

}  // namespace clover2_led
