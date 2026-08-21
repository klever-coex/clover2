#pragma once

#include <clover2_common/node_context.hpp>
#include <clover2_notification/data/event.hpp>
#include <rclcpp/rclcpp.hpp>

#include <functional>
#include <memory>

namespace clover2_notification::provider {

using callback_type = std::function<void(const data::event&)>;

class base {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(base)

    virtual ~base() = default;

    virtual void initialize(
        std::shared_ptr<clover2_common::node_context> node_context,
        callback_type callback) = 0;
    virtual void cleanup() = 0;
};

}  // namespace clover2_notification::provider
