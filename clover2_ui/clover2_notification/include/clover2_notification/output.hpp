#pragma once

#include <clover2_notification/data/event.hpp>

#include <rclcpp_lifecycle/lifecycle_node.hpp>

#include <memory>

namespace clover2_notification {

class output {
public:
    virtual ~output() = default;

    virtual void initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) = 0;
    virtual void show(const data::event& event) = 0;
    virtual void clear() = 0;
};

}  // namespace clover2_notification
