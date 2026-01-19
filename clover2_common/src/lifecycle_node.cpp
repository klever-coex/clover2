#include <clover2_common/lifecycle_node.hpp>
#include <lifecycle_msgs/msg/state.hpp>

namespace clover2_common {

lifecycle_node::lifecycle_node(const std::string& node_name,
                               const rclcpp::NodeOptions& options)
    : rclcpp_lifecycle::LifecycleNode(node_name, options)
    , m_diagnostic_updater(nullptr) {
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

lifecycle_node::SetParametersResult lifecycle_node::on_set_parameters_cb(
    const std::vector<rclcpp::Parameter>& parameters) {
    SetParametersResult result;
    result.successful = true;

    for (auto& p : parameters) {
        auto it = m_watch_parameters.find(p.get_name());
        if (it != m_watch_parameters.end()) {
            try {
                it->second(p);
            } catch (std::exception& ex) {
                result.successful = false;
                result.reason = ex.what();
                RCLCPP_ERROR(get_logger(), "Fail set parameter `%s` with: %s",
                             p.get_name().c_str(), ex.what());
                break;
            }
        }
    }

    return result;
}

void lifecycle_node::enable_watch_parameters() {
    m_set_parameters_handle_ptr = add_on_set_parameters_callback(std::bind(
        &lifecycle_node::on_set_parameters_cb, this, std::placeholders::_1));
}

void lifecycle_node::enable_diagnostic_updater() {
    m_diagnostic_updater = std::make_shared<diagnostic_updater::Updater>(this);
    m_diagnostic_updater->setHardwareID(this->get_name());

    m_diagnostic_updater->add(
        "Lifecycle State",
        std::bind(&lifecycle_node::produce_lifecycle_diagnostics, this,
                  std::placeholders::_1));
}

std::shared_ptr<diagnostic_updater::Updater>
lifecycle_node::get_diagnostic_updater() {
    return m_diagnostic_updater;
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

}  // namespace clover2_common
