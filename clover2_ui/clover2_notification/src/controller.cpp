#include <clover2_notification/controller.hpp>

#include <diagnostic_msgs/msg/diagnostic_status.hpp>

#include <exception>
#include <functional>

namespace clover2_notification {

controller::controller(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("notification", options) {
    m_output_plugins = declare_parameter<std::vector<std::string>>(
        "outputs", {"clover2_notification::outputs::led"});
    m_repeat_period = rclcpp::Duration::from_seconds(
        declare_parameter<double>("repeat_period_sec", 0.0));

    register_on_configure(
        std::bind(&controller::on_configure, this, std::placeholders::_1));
    register_on_activate(
        std::bind(&controller::on_activate, this, std::placeholders::_1));
    register_on_deactivate(
        std::bind(&controller::on_deactivate, this, std::placeholders::_1));
    register_on_cleanup(
        std::bind(&controller::on_cleanup, this, std::placeholders::_1));
    register_on_shutdown(
        std::bind(&controller::on_shutdown, this, std::placeholders::_1));
}

controller::~controller() = default;

controller::CallbackReturn controller::on_configure(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    try {
        m_node_context = std::make_shared<clover2_common::node_context>(*this);
        auto node = shared_from_this();
        for (const auto& plugin_name : m_output_plugins) {
            auto plugin = m_output_loader.createSharedInstance(plugin_name);
            plugin->initialize(node);
            m_outputs.emplace_back(std::move(plugin));
            RCLCPP_INFO(get_logger(), "Loaded notification output: %s",
                        plugin_name.c_str());
        }
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Failed to load notification output: %s",
                     e.what());
        m_outputs.clear();
        m_node_context.reset();
        return CallbackReturn::FAILURE;
    }

    return CallbackReturn::SUCCESS;
}

controller::CallbackReturn controller::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_diagnostics_sub =
        rclcpp::create_subscription<diagnostic_msgs::msg::DiagnosticArray>(
            m_node_context, "/diagnostics_agg", rclcpp::QoS(10),
            std::bind(&controller::diagnostics_callback, this,
                      std::placeholders::_1));
    return CallbackReturn::SUCCESS;
}

controller::CallbackReturn controller::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_diagnostics_sub.reset();
    for (const auto& output : m_outputs) {
        output->clear();
    }
    return CallbackReturn::SUCCESS;
}

controller::CallbackReturn controller::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_outputs.clear();
    m_node_context.reset();
    return CallbackReturn::SUCCESS;
}

controller::CallbackReturn controller::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    m_diagnostics_sub.reset();
    m_outputs.clear();
    m_node_context.reset();
    return CallbackReturn::SUCCESS;
}

void controller::diagnostics_callback(
    diagnostic_msgs::msg::DiagnosticArray::SharedPtr msg) {
    const diagnostic_msgs::msg::DiagnosticStatus* selected_status = nullptr;
    uint8_t selected_level = diagnostic_msgs::msg::DiagnosticStatus::OK;

    for (const auto& status : msg->status) {
        if (status.level > selected_level) {
            selected_level = status.level;
            selected_status = &status;
        }
    }

    if (!selected_status) {
        m_active_event.value.reset();
        return;
    }

    data::event event{*selected_status};
    const auto now = get_clock()->now();
    const bool event_changed =
        !m_active_event.value || *m_active_event.value != event;
    const bool repeat_due =
        !event_changed && m_repeat_period.nanoseconds() > 0 &&
        (now - m_active_event.stamp) >= m_repeat_period;

    if (!event_changed && !repeat_due) {
        return;
    }

    m_active_event.value = event;
    m_active_event.stamp = now;

    for (const auto& output : m_outputs) {
        output->show(event);
    }
}

}  // namespace clover2_notification
