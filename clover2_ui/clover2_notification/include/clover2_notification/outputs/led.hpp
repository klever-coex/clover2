#pragma once

#include <clover2_led/client.hpp>
#include <clover2_notification/output.hpp>

#include <memory>
#include <string>

namespace clover2_notification::outputs {

class led final : public clover2_notification::output {
public:
    led() = default;
    ~led() override = default;

    void initialize(
        const rclcpp_lifecycle::LifecycleNode::SharedPtr& node) override;
    void show(const std::string& notification_name) override;
    void clear() override;

private:
    std::shared_ptr<clover2_led::client> m_client;
};

}  // namespace clover2_notification::outputs
