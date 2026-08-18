// clover2
#include <clover2_common/diagnostics/lifecycle_state_task.hpp>
#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_interfaces/node_diagnostics.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher.hpp>

// msgs
#include <lifecycle_msgs/msg/state.hpp>

#include <memory>
#include <utility>

namespace clover2_common {

lifecycle_node::lifecycle_node(const std::string& node_name,
                               const rclcpp::NodeOptions& options)
    : rclcpp_lifecycle::LifecycleNode(node_name, options)
    , m_diagnostics(std::make_shared<node_interfaces::NodeDiagnostics>(
          get_node_base_interface(), get_node_clock_interface(),
          get_node_logging_interface(), get_node_parameters_interface(),
          get_node_timers_interface(), get_node_topics_interface()))
    , m_parameters_watcher(
          std::make_shared<node_interfaces::NodeParametersWatcher>(
              get_node_parameters_interface())) {
    init_lifecycle_node();
}

void lifecycle_node::init_lifecycle_node() {
    get_node_diagnostics_interface()->add<diagnostics::lifecycle_state_task>();
    get_node_diagnostics_interface()
        ->get<diagnostics::lifecycle_state_task>()
        .set_state_getter([this]() { return get_current_state(); });

    declare_parameter("autostart", true);

    if (get_parameter("autostart").as_bool()) {
        m_init_timer =
            this->create_wall_timer(std::chrono::seconds(0), [this]() {
                configure();

                if (get_current_state().id() ==
                    lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE) {
                    activate();
                }

                m_init_timer.reset();
            });
    }
}

lifecycle_node::~lifecycle_node() { RCLCPP_INFO(get_logger(), "Destroying"); }

clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
lifecycle_node::get_node_diagnostics_interface() {
    return m_diagnostics;
}

clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
lifecycle_node::get_node_parameters_watcher_interface() {
    return m_parameters_watcher;
}

}  // namespace clover2_common
