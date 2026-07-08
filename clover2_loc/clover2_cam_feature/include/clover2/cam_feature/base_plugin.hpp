#pragma once

// clover2
#include <clover2/map/client.hpp>
#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_context.hpp>

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

    void configure(
        const std::string& name,
        const std::shared_ptr<clover2_common::node_context>& node_context,
        const std::shared_ptr<clover2::map::client>& map_client);
    void activate();
    void deactivate();
    void cleanup();

    virtual std::list<clover2_pose_msgs::msg::Marker> process(
        const std_msgs::msg::Header& header, const cv::Mat& image,
        const cv::Matx33d& matrix, const cv::Mat_<double>& distortion,
        std::shared_ptr<cv::Mat> debug = nullptr) = 0;

protected:
    virtual void on_configure(
        const std::string& name,
        const std::shared_ptr<clover2_common::node_context>& node,
        const std::shared_ptr<clover2::map::client>& map_client) = 0;
    virtual void on_activate() = 0;
    virtual void on_deactivate() = 0;
    virtual void on_cleanup() = 0;

    const std::string& get_name() const;

    rclcpp::Logger get_logger() const;
    rclcpp::Clock::SharedPtr get_clock() const;

    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        clover2_common::util::ParameterFunctorT cb,
        const std::string& description = "",
        const std::string& additional_constraints = "", bool read_only = false,
        bool ignore_override = false) {
        clover2_common::util::declare_and_watch_parameter<ParameterT>(
            m_parameters_watcher, name, default_value, cb,
            description, additional_constraints, read_only, ignore_override);
    }

    std::string m_name;
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
    std::shared_ptr<clover2_common::node_context> m_node_context;
    clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
        m_parameters_watcher;
};

}  // namespace clover2::cam_feature
