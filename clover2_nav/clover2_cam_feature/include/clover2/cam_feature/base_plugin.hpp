#pragma once

// clover2
#include <clover2/cam_feature/plugin_context.hpp>
#include <clover2/common/parameter_watcher.hpp>
#include <clover2/map/client.hpp>

// rclcpp
#include <opencv2/core/cvstd_wrapper.hpp>
#include <rclcpp/clock.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>

// OpenCV
#include <opencv2/core.hpp>

// msgs
#include <clover2_pose_msgs/msg/marker.hpp>

// STL
#include <memory>
#include <string>
#include <vector>

#define CAM_FEATURE_PLUGIN_REGISTER(class)                    \
    CLASS_LOADER_REGISTER_CLASS(                              \
        clover2::cam_feature::plugin_factory_template<class>, \
        clover2::cam_feature::plugin_factory)

namespace clover2::cam_feature {

class base_plugin {
public:
    RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(base_plugin)

    using ParameterFunctorT =
        clover2::common::parameter_watcher::ParameterFunctorT;

    explicit base_plugin(
        clover2::cam_feature::plugin_context& ctx, const std::string& subnode,
        const rclcpp::NodeOptions& options = rclcpp::NodeOptions())
        : m_node(std::move(ctx.node))
        , m_parameter_watcher(*m_node) {}

    virtual ~base_plugin() = default;

    virtual std::list<clover2_pose_msgs::msg::Marker> process(
        const std_msgs::msg::Header& header, const cv::Mat& image,
        const cv::Matx33d& matrix, const cv::Mat_<double>& distortion,
        std::shared_ptr<cv::Mat> debug = nullptr) = 0;

    rclcpp::Node::SharedPtr get_node() const { return m_node; }

protected:
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
};

class plugin_factory {
public:
    plugin_factory() = default;
    virtual ~plugin_factory() = default;

    virtual base_plugin::SharedPtr create_plugin_instance(
        clover2::cam_feature::plugin_context& ctx) = 0;
};

template <typename PluginT>
class plugin_factory_template : public plugin_factory {
public:
    plugin_factory_template() = default;
    virtual ~plugin_factory_template() = default;

    base_plugin::SharedPtr create_plugin_instance(
        clover2::cam_feature::plugin_context& ctx) override {
        return std::make_shared<PluginT>(ctx);
    }
};

}  // namespace clover2::cam_feature
