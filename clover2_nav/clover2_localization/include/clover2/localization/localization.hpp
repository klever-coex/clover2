#pragma once

#include <clover2/localization/data/sensor_data.hpp>
#include <clover2/localization/graph/graph_manager.hpp>
#include <clover2/localization/handler/aruco_handler.hpp>
#include <clover2/localization/queue/frame_queue.hpp>
#include <clover2/localization/sensor/base_sensor.hpp>

#include <clover2/common/lifecycle_node.hpp>

#include <geometry_msgs/msg/pose_stamped.hpp>
#include <rclcpp/rclcpp.hpp>

#include <atomic>
#include <memory>
#include <string>
#include <thread>

namespace clover2::localization {

class localization : public clover2::common::lifecycle_node {
public:
    using CallbackReturn =
        rclcpp_lifecycle::node_interfaces::LifecycleNodeInterface::CallbackReturn;

    explicit localization(
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions());
    ~localization() override;

    CallbackReturn on_configure(const rclcpp_lifecycle::State& state) override;
    CallbackReturn on_activate(const rclcpp_lifecycle::State& state) override;
    CallbackReturn on_deactivate(const rclcpp_lifecycle::State& state) override;
    CallbackReturn on_cleanup(const rclcpp_lifecycle::State& state) override;
    CallbackReturn on_shutdown(const rclcpp_lifecycle::State& state) override;

    rclcpp::node_interfaces::NodeBaseInterface::SharedPtr
    get_sensor_node_base_interface() const;

private:
    void sensor_callback(const data::sensor_data& data);
    void graph_thread_func();

    std::string m_sensor_type;
    std::string m_handler_type;
    double m_opt_frequency;
    int m_opt_iterations;

    std::unique_ptr<handler::base_handler> m_handler;
    std::shared_ptr<sensor::base_sensor> m_sensor;
    rclcpp::Node::SharedPtr m_sensor_node;
    std::shared_ptr<queue::frame_queue> m_frame_queue;
    std::shared_ptr<graph::graph_manager> m_graph_manager;

    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr m_pose_pub;

    std::thread m_graph_thread;
    std::atomic<bool> m_running{false};
};

}  // namespace clover2::localization
