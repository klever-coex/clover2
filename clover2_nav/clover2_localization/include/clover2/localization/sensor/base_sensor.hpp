#pragma once

// clover2
#include <clover2/common/parameter_watcher.hpp>
#include <clover2/localization/sensor/creation_context.hpp>

// rclcpp
#include <rclcpp/clock.hpp>
#include <rclcpp/logger.hpp>
#include <rclcpp/rclcpp.hpp>

// STL
#include <string>

namespace clover2::localization::sensor {
class base_sensor : public std::enable_shared_from_this<base_sensor>,
                    public clover2::common::parameter_watcher {
public:
    RCLCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(base_sensor)

    explicit base_sensor(creation_context& ctx, const std::string& subnode)
        : m_node(std::make_shared<rclcpp::Node>(
              subnode, ctx.node_base->get_fully_qualified_name()))
        , m_parameter_watcher(*m_node) {}

    virtual ~base_sensor() = default;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual rclcpp::Node::SharedPtr get_node() const { return m_node; }

    virtual rclcpp::Logger get_logger() const { return m_node->get_logger(); }

    virtual rclcpp::Clock::SharedPtr get_clock() const {
        return m_node->get_clock();
    }

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

private:
    rclcpp::Node::SharedPtr m_node;
    clover2::common::parameter_watcher m_parameter_watcher;
};

}  // namespace clover2::localization::sensor
