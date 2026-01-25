#pragma once

#include <rclcpp/rclcpp.hpp>

namespace clover2::backend::adapter {

class topic {
public:
    topic() = default;
    virtual ~topic() = default;
};

}  // namespace clover2::backend::adapter
