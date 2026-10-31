#pragma once

// clover2
#include <clover2_common/node.hpp>
#include <clover2_fcu/backend/backend_base.hpp>
#include <clover2_fcu_msgs/msg/mode_state.hpp>
#include <clover2_fcu_msgs/srv/activate_mode.hpp>
#include <clover2_fcu_msgs/srv/get_config.hpp>
#include <clover2_fcu_msgs/srv/register_mode.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// third party
#include <bondcpp/bond.hpp>
#include <pluginlib/class_loader.hpp>

// STL
#include <map>
#include <memory>
#include <optional>
#include <string>

namespace clover2_fcu {

class bridge : public clover2_common::node {
public:
    explicit bridge(const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~bridge() override;

private:
    void handle_get_config(
        const std::shared_ptr<clover2_fcu_msgs::srv::GetConfig::Request>
            request,
        std::shared_ptr<clover2_fcu_msgs::srv::GetConfig::Response> response);

    void handle_register_mode(
        const std::shared_ptr<clover2_fcu_msgs::srv::RegisterMode::Request>
            request,
        std::shared_ptr<clover2_fcu_msgs::srv::RegisterMode::Response>
            response);

    void handle_activate_mode(
        const std::shared_ptr<clover2_fcu_msgs::srv::ActivateMode::Request>
            request,
        std::shared_ptr<clover2_fcu_msgs::srv::ActivateMode::Response>
            response);

    void handle_mode_bond_broken(uint32_t mode_id);

    /// @brief Publish the current lease on mode_state.
    void publish_mode_state();

    struct mode_record {
        std::string name;
        std::shared_ptr<bond::Bond> bond;
        bool broken{false};
        std::optional<uint32_t> native_mode_id{0};
    };

    rclcpp::Service<clover2_fcu_msgs::srv::GetConfig>::SharedPtr
        m_get_config_service;
    rclcpp::Service<clover2_fcu_msgs::srv::RegisterMode>::SharedPtr
        m_register_mode_service;
    rclcpp::Service<clover2_fcu_msgs::srv::ActivateMode>::SharedPtr
        m_activate_mode_service;

    rclcpp::Publisher<clover2_fcu_msgs::msg::ModeState>::SharedPtr
        m_mode_state_pub;

    backend::backend_base::shared_ptr m_factory;
    pluginlib::ClassLoader<backend::backend_base> m_loader{
        "clover2_fcu", "clover2_fcu::backend::backend_base"};

    clover2_fcu_msgs::msg::FcuConfig m_config;

    std::map<uint32_t, mode_record> m_modes;
    uint32_t m_next_mode_id{1};
    uint32_t m_active_mode_id{0};
};

}  // namespace clover2_fcu
