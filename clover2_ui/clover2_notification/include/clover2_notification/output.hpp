#pragma once

#include <rclcpp_lifecycle/lifecycle_node.hpp>

#include <memory>
#include <string>

namespace clover2_notification {

class output {
public:
    virtual ~output() = default;

    virtual void initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) = 0;
    virtual void show(const std::string& notification_name) = 0;
    virtual void clear() = 0;
};

}  // namespace clover2_notification
