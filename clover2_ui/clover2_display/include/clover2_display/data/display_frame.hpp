#pragma once

#include <cv_bridge/cv_bridge.hpp>
#include <opencv2/core.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <stdexcept>
#include <string>

namespace clover2_display::data {

struct display_frame {
    std::string encoding;
    cv::Mat image;

    display_frame() = default;
    explicit display_frame(const sensor_msgs::msg::Image& msg)
        : encoding(msg.encoding)
        , image(cv::Mat(static_cast<int>(msg.height),
                        static_cast<int>(msg.width),
                        cv_bridge::getCvType(msg.encoding),
                        const_cast<uint8_t*>(msg.data.data()), msg.step)
                    .clone()) {}

    int width() const noexcept { return image.cols; }
    int height() const noexcept { return image.rows; }
};

}  // namespace clover2_display::data
