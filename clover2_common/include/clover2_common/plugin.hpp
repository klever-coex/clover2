#pragma once

// clover2
#include <clover2_common/node_context.hpp>

// ROS2
#include <rclcpp/macros.hpp>

namespace clover2_common {

class plugin {
public:
    RCLCPP_DISABLE_COPY(plugin)

    virtual ~plugin() = default;

    void initialize(const std::string& name,
                    std::shared_ptr<clover2_common::node_context> node_context);
    void cleanup() noexcept;

    std::string_view get_name() const;

protected:
    explicit plugin();

    rclcpp::Logger get_logger() const;
    rclcpp::Clock::SharedPtr get_clock() const;

    virtual void on_initialize();
    virtual void on_cleanup() noexcept;

    std::shared_ptr<clover2_common::node_context> m_node_context;

    template <typename ParameterT>
    void declare_and_watch_parameter(
        const std::string& name, const ParameterT& default_value,
        parameter_watcher::ParameterFunctorT cb,
        const std::string& description = "",
        const std::string& additional_constraints = "", bool read_only = false,
        bool ignore_override = false) {
        if (!m_parameter_watcher) {
            throw std::runtime_error("Parameter watcher not created");
        }

        m_parameter_watcher->declare_and_watch_parameter(
            name, default_value, cb, description, additional_constraints,
            read_only, ignore_override);
    }

private:
    std::string m_name;
    rclcpp::Logger m_logger;
    rclcpp::Clock::SharedPtr m_clock;
};

}  // namespace clover2_common
