#pragma once

// clover2
#include <clover2_fcu/capability/parameters_interface.hpp>
#include <clover2_fcu/context.hpp>

// ROS2
#include <rclcpp/client.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/subscription.hpp>
#include <rclcpp/timer.hpp>

// msgs
#include <mavros_msgs/msg/param_event.hpp>
#include <mavros_msgs/srv/param_pull.hpp>
#include <mavros_msgs/srv/param_set_v2.hpp>

// STL
#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

namespace clover2_fcu_mavros::capability {

using clover2_fcu::result;

class parameters : public clover2_fcu::capability::parameters_interface,
                   public std::enable_shared_from_this<parameters> {
public:
    explicit parameters(const clover2_fcu::context& ctx, std::string prefix);

    result get(const std::string& name,
               rclcpp::ParameterValue& out) override final;
    result set(const std::string& name,
               const rclcpp::ParameterValue& value) override final;

    std::vector<std::string> list() const override final;

private:
    void attempt_pull();
    void handle_event(const mavros_msgs::msg::ParamEvent& msg);

    std::optional<rclcpp::ParameterValue> cached(const std::string& name) const;
    void store(const std::string& name, rclcpp::ParameterValue value);

    rclcpp::Logger m_logger;
    rclcpp::CallbackGroup::SharedPtr m_params_group;
    rclcpp::Subscription<mavros_msgs::msg::ParamEvent>::SharedPtr m_event_sub;
    rclcpp::Client<mavros_msgs::srv::ParamPull>::SharedPtr m_pull_client;
    rclcpp::Client<mavros_msgs::srv::ParamSetV2>::SharedPtr m_set_client;
    std::function<rclcpp::TimerBase::SharedPtr(rclcpp::Duration)>
        m_timer_factory;
    rclcpp::TimerBase::SharedPtr m_retry_timer;

    std::chrono::seconds m_call_timeout{5};

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, rclcpp::ParameterValue> m_cache;
    bool m_pulled{false};
};

}  // namespace clover2_fcu_mavros::capability
