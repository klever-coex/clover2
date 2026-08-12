#include "clover2_common/diagnostics/lifecycle_state_task.hpp"

#include <lifecycle_msgs/msg/state.hpp>

namespace clover2_common::diagnostics {

void lifecycle_state_task::set_state_getter(state_getter_fn getter) {
    m_state_getter = getter;
}

void lifecycle_state_task::run(
    diagnostic_updater::DiagnosticStatusWrapper& stat) {
    if (!m_state_getter) {
        stat.summary(diagnostic_msgs::msg::DiagnosticStatus::ERROR,
                     "Lifecycle state getter is not set");
        return;
    }
    uint8_t level = diagnostic_msgs::msg::DiagnosticStatus::ERROR;

    const auto state = m_state_getter();

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

    stat.summaryf(level, "Lifecycle State: %s", state.label().c_str());
}

}  // namespace clover2_common::diagnostics
