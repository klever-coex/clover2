// clover2
#include <clover2_common/lifecycle_node.hpp>
#include <clover2_common/node_interfaces/node_diagnostics.hpp>
#include <clover2_common/node_interfaces/node_parameters_watcher.hpp>

// msgs
#include <lifecycle_msgs/msg/state.hpp>

#include <memory>

namespace clover2_common {

lifecycle_node::lifecycle_node(const std::string& node_name,
                               const rclcpp::NodeOptions& options)
    : rclcpp_lifecycle::LifecycleNode(node_name, options)
    , m_parameters_watcher(new node_interfaces::NodeParametersWatcher(
          get_node_parameters_interface())) {
    declare_parameter("autostart", true);
    enable_diagnostic_updater();

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

void lifecycle_node::enable_diagnostic_updater() {
    if (m_diagnostics) {
        return;
    }

    m_diagnostics = std::make_shared<node_interfaces::NodeDiagnostics>(
        get_node_base_interface(), get_node_clock_interface(),
        get_node_logging_interface(), get_node_parameters_interface(),
        get_node_timers_interface(), get_node_topics_interface());

    add_lifecycle_diagnostics();
}

void lifecycle_node::add_lifecycle_diagnostics() {
    m_diagnostics->remove_by_name("Lifecycle State");
    m_diagnostics->add(
        "Lifecycle State",
        std::bind(&lifecycle_node::produce_lifecycle_diagnostics, this,
                  std::placeholders::_1));
}

void lifecycle_node::set_node_diagnostics_interface(
    clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
        diagnostics) {
    m_diagnostics = diagnostics;
    add_lifecycle_diagnostics();
}

void lifecycle_node::produce_lifecycle_diagnostics(
    diagnostic_updater::DiagnosticStatusWrapper& status) {
    uint8_t level = diagnostic_msgs::msg::DiagnosticStatus::ERROR;
    auto& state = this->get_current_state();

    switch (state.id()) {
        case lifecycle_msgs::msg::State::PRIMARY_STATE_UNKNOWN:
        case lifecycle_msgs::msg::State::PRIMARY_STATE_INACTIVE:
            level = diagnostic_msgs::msg::DiagnosticStatus::WARN;
            break;
        case lifecycle_msgs::msg::State::PRIMARY_STATE_UNCONFIGURED:
        case lifecycle_msgs::msg::State::PRIMARY_STATE_FINALIZED:
        case lifecycle_msgs::msg::State::TRANSITION_STATE_CONFIGURING:
        case lifecycle_msgs::msg::State::TRANSITION_STATE_CLEANINGUP:
        case lifecycle_msgs::msg::State::TRANSITION_STATE_SHUTTINGDOWN:
        case lifecycle_msgs::msg::State::TRANSITION_STATE_ACTIVATING:
        case lifecycle_msgs::msg::State::TRANSITION_STATE_DEACTIVATING:
            level = diagnostic_msgs::msg::DiagnosticStatus::STALE;
            break;
        case lifecycle_msgs::msg::State::PRIMARY_STATE_ACTIVE:
            level = diagnostic_msgs::msg::DiagnosticStatus::OK;
            break;
        case lifecycle_msgs::msg::State::TRANSITION_STATE_ERRORPROCESSING:
            level = diagnostic_msgs::msg::DiagnosticStatus::ERROR;
            break;
        default:
            level = diagnostic_msgs::msg::DiagnosticStatus::ERROR;
            break;
    }

    status.summaryf(level, "Lifecycle State: %s", state.label().c_str());
}

clover2_common::node_interfaces::NodeDiagnosticsInterface::SharedPtr
lifecycle_node::get_node_diagnostics_interface() {
    return m_diagnostics;
}

clover2_common::node_interfaces::NodeParametersWatcherInterface::SharedPtr
lifecycle_node::get_node_parameters_watcher_interface() {
    return m_parameters_watcher;
}

}  // namespace clover2_common
