#pragma once

#include <clover2/localization/data/frame_data.hpp>
#include <clover2/localization/data/sensor_data.hpp>

#include <memory>

namespace clover2::localization::handler {

class base_handler {
public:
    virtual ~base_handler() = default;

    virtual data::frame_data process(const data::sensor_data& data) = 0;
};

}  // namespace clover2::localization::handler
