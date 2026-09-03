#pragma once

#include <clover2_common/node.hpp>

namespace clover2_thermal {

class camera : public clover2_common::node {
public:
    explicit camera(const rclcpp::NodeOptions& options);
    ~camera();

private:
};

}  // namespace clover2_thermal
