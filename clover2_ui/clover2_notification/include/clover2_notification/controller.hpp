/**
 * @file controller.hpp
 * @brief Provides the notification controller node.
 */

#pragma once

// clover2
#include <clover2_common/node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2_notification/data/event.hpp>
#include <clover2_notification/output.hpp>
#include <clover2_notification/provider/base.hpp>

// ROS2
#include <pluginlib/class_loader.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <memory>
#include <string>
#include <vector>

namespace clover2_notification {

/**
 * @class controller
 * @brief Node that routes notification events from providers to
 * output plugins.
 *
 * The controller loads configured notification providers and output plugins
 * during construction. Providers report notification events through
 * a callback, and the controller forwards each event to every configured
 * output.
 */
class controller : public clover2_common::node {
public:
    RCLCPP_SMART_PTR_DEFINITIONS(controller)

    /**
     * @brief Construct the notification controller node.
     *
     * Declares controller parameters and initializes providers and outputs.
     *
     * @param options ROS 2 node options.
     */
    explicit controller(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());

    /** @brief Destroy the notification controller node. */
    ~controller() override;

private:
    /**
     * @brief Forward a provider event to all configured outputs.
     *
     * @param event Notification event produced by a provider.
     */
    void provider_callback(const data::event& event);

    pluginlib::ClassLoader<output> m_output_loader{
        "clover2_notification", "clover2_notification::output"};
    std::vector<std::string> m_provider_names;
    std::vector<std::string> m_output_ids;
    std::vector<std::shared_ptr<provider::base>> m_providers;
    std::vector<std::shared_ptr<output>> m_outputs;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace clover2_notification
