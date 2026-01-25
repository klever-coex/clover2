#pragma once

#include <rclcpp/rclcpp.hpp>

#include <memory>

namespace clover2::backend {

// forward declaration
class app;

class server : public rclcpp::Node {
public:
    explicit server(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    virtual ~server();

private:
    void init_app();

    std::shared_ptr<app> m_app;
    std::shared_ptr<rclcpp::TimerBase> m_init_timer;
};

}  // namespace clover2::backend
