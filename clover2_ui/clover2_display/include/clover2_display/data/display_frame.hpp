#pragma once

#include <opencv2/core.hpp>
#include <sensor_msgs/msg/image.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

namespace clover2_display::data {

inline int encoding_to_cv_type(const std::string& encoding) {
    if (encoding == "mono8") {
        return CV_8UC1;
    }

    throw std::runtime_error("display_frame: unsupported encoding '" +
                             encoding + "'");
}

struct display_frame {
    std::string encoding;
    cv::Mat image;

    display_frame() = default;
    explicit display_frame(const sensor_msgs::msg::Image& msg)
        : encoding(msg.encoding)
        , image(cv::Mat(static_cast<int>(msg.height),
                        static_cast<int>(msg.width),
                        encoding_to_cv_type(msg.encoding),
                        const_cast<uint8_t*>(msg.data.data()), msg.step)
                    .clone()) {}

    int width() const noexcept { return image.cols; }
    int height() const noexcept { return image.rows; }
};

}  // namespace clover2_display::data
