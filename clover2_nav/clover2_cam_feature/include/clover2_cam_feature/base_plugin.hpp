#pragma once

// clover2
#include <clover2/map_server/map_client.hpp>
#include <clover2/common/parameter_watcher.hpp>
#include <clover2_cam_feature/plugin_context.hpp>

// rclcpp
#include <opencv2/core/cvstd_wrapper.hpp>
#include <rclcpp/clock.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>

// OpenCV
#include <opencv2/core.hpp>

// msgs
#include <clover2_localization_msgs/msg/feature3.hpp>

// STL
#include <memory>
#include <string>
#include <vector>

namespace clover2_cam_feature {

class base_plugin {
public:
    RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(base_plugin)

    using ParameterFunctorT =
        clover2::common::parameter_watcher::ParameterFunctorT;

    base_plugin() = default;
    virtual ~base_plugin() = default;

    void init_plugin(const std::string& subnode,
                     const clover2_cam_feature::plugin_context& ctx) {
        m_node = std::make_shared<rclcpp::Node>(
            subnode, ctx.node_base->get_fully_qualified_name());
        m_parameter_watcher.enable_watch_parameters(*m_node);
        m_map_client = ctx.map_client;

        init(ctx);
    }

    virtual std::list<clover2_localization_msgs::msg::Feature3> process(
        const cv::Mat& image, const cv::Matx33d& matrix,
        const cv::Mat_<double>& distortion,
        std::shared_ptr<cv::Mat> debug = nullptr) = 0;

protected:
    virtual void init(const clover2_cam_feature::plugin_context& ctx) = 0;

    rclcpp::Node::SharedPtr get_node() const { return m_node; }

    rclcpp::Logger get_logger() const { return m_node->get_logger(); }

    rclcpp::Clock::SharedPtr get_clock() const { return m_node->get_clock(); }

    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        ParameterFunctorT cb, const std::string& description = "",
        const std::string& additional_constraints = "", bool read_only = false,
        bool ignore_override = false) {
        m_parameter_watcher.declare_and_watch_parameter(
            name, default_value, cb, description, additional_constraints,
            read_only, ignore_override);
    }

    rclcpp::Node::SharedPtr m_node;
    clover2::common::parameter_watcher m_parameter_watcher;

    std::shared_ptr<clover2::map_server::map_client> m_map_client;
};

}  // namespace clover2_cam_feature
