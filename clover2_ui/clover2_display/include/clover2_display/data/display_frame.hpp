#pragma once

#include <sensor_msgs/msg/image.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace clover2_display::data {

struct display_frame {
    // Internal frame model copied from sensor_msgs/Image by the driver.
    // TODO: Add validation helpers.
    uint32_t width{0};
    uint32_t height{0};
    std::string encoding;
    uint32_t step{0};
    std::vector<uint8_t> data;

    display_frame() = default;
    explicit display_frame(const sensor_msgs::msg::Image& msg)
        : width(msg.width)
        , height(msg.height)
        , encoding(msg.encoding)
        , step(msg.step)
        , data(msg.data) {}
};

}  // namespace clover2_display::data
