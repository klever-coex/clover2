#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace clover2_display::data {

struct display_info {
    uint32_t width{128};
    uint32_t height{64};
    double max_fps{10.0};
    std::vector<std::string> supported_encodings{"mono8"};
};

}  // namespace clover2_display::data
