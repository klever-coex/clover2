#pragma once

// clover2
#include <clover2/localization/data/sensor_data.hpp>
#include <clover2/localization/sensor/creation_context.hpp>
#include <clover2/common//

// rclcpp
#include <rclcpp/node.hpp>

// STL
#include <functional>
#include <string>

namespace clover2::localization::sensor {

using sensor_callback = std::function<void(const data::sensor_data&)>;

struct sensor_params {
    int32_t sensor_id = 0;
    std::string image_topic = "~/image_raw";
    std::string camera_info_topic = "~/camera_info";
};

class base_sensor {
public:
    explicit base_sensor(creation_context& ctx);
    virtual ~base_sensor() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    creation_context ctx;
};

}  // namespace clover2::localization::sensor
