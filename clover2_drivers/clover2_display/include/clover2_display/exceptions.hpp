#pragma once

#include <stdexcept>
#include <string>

namespace clover2_display {

class exception : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

namespace data {

class frequency_to_high : public clover2_display::exception {
public:
    frequency_to_high()
        : clover2_display::exception("Data rate is too high") {}
};

class empty_frame : public clover2_display::exception {
public:
    empty_frame()
        : clover2_display::exception("display frame is empty") {}
};

class unsupported_encoding : public clover2_display::exception {
public:
    explicit unsupported_encoding(const std::string& encoding)
        : clover2_display::exception("unsupported display frame encoding: " +
                                     encoding) {}
};

class encoding_type_mismatch : public clover2_display::exception {
public:
    explicit encoding_type_mismatch(const std::string& encoding)
        : clover2_display::exception(
              "image matrix type does not match encoding: " + encoding) {}
};

}  // namespace data

namespace device {

class frame_size_mismatch : public clover2_display::exception {
public:
    frame_size_mismatch(int expected_width, int expected_height,
                        int actual_width, int actual_height)
        : clover2_display::exception(
              "display frame size mismatch: expected " +
              std::to_string(expected_width) + "x" +
              std::to_string(expected_height) + ", got " +
              std::to_string(actual_width) + "x" +
              std::to_string(actual_height)) {}
};

}  // namespace device

}  // namespace clover2_display
