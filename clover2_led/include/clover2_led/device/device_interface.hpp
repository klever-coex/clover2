#pragma once

#include <clover2_led/data/driver_info.hpp>
#include <clover2_led/msg/color.hpp>

#include <span>

namespace clover2_led::device {

class device_interface {
public:
    device_interface();
    virtual ~device_interface() = default;

    virtual const clover2_led::data::driver_info& info() const = 0;

    virtual bool initialize() = 0;

    virtual void shutdown() = 0;

    virtual bool write_raw_frame(std::span<clover2_led::data::color> frame) = 0;
};

}  // namespace clover2_led::device
