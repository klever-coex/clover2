#pragma once

// clover2
#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_context.hpp>
#include <clover2/map/client.hpp>

// rclcpp
#include <rclcpp/clock.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>

// OpenCV
#include <opencv2/core.hpp>

// msgs
#include <clover2_pose_msgs/msg/marker.hpp>

// STL
#include <memory>
#include <stdexcept>
#include <string>

#define CAM_FEATURE_PLUGIN_REGISTER(class) \
    CLASS_LOADER_REGISTER_CLASS(class, clover2::cam_feature::base_plugin)

namespace clover2::cam_feature {

class base_plugin {
public:
    RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(base_plugin)

    explicit base_plugin();
    virtual ~base_plugin();

    void configure(const std::string& name,
                   rclcpp_lifecycle::LifecycleNode::WeakPtr node,
                   const std::shared_ptr<clover2::map::client>& map_client) {
        auto node_ptr = node.lock();
        if (!node_ptr) {
            throw std::runtime_error("Fail to lock node ptr");
        }

        m_name = name;

        m_node_context =
            std::make_shared<clover2_common::node_context>(*node_ptr);

        m_logger = m_node_context->get_node_logging_interface()
                       ->get_logger()
                       .get_child(name);
        m_clock = m_node_context->get_node_clock_interface()->get_clock();

        on_configure(name, node, map_client);
        RCLCPP_DEBUG(get_logger(), "Configured");
    }

    void activate() { on_activate(); }
    void deactivate() { on_deactivate(); }

    void cleanup() {
        on_cleanup();
        m_node_context.reset();

        RCLCPP_DEBUG(get_logger(), "Cleaned up");
    }

    virtual std::list<clover2_pose_msgs::msg::Marker> process(
        const std_msgs::msg::Header& header, const cv::Mat& image,
        const cv::Matx33d& matrix, const cv::Mat_<double>& distortion,
        std::shared_ptr<cv::Mat> debug = nullptr) = 0;

protected:
    virtual void on_configure(
        const std::string& name,
        const rclcpp_lifecycle::LifecycleNode::WeakPtr& node,
        const std::shared_ptr<clover2::map::client>& map_client) = 0;
    virtual void on_activate() = 0;
    virtual void on_deactivate() = 0;
    virtual void on_cleanup() = 0;

    const std::string& get_name() const { return m_name; }

    rclcpp::Logger get_logger() const { return m_logger; }
    rclcpp::Clock::SharedPtr get_clock() const { return m_clock; }

    std::string m_name;
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
    std::shared_ptr<clover2_common::node_context> m_node_context;
};

}  // namespace clover2::cam_feature
