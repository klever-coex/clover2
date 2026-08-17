#include <clover2_notification/controller.hpp>

#include <exception>
#include <functional>

namespace clover2_notification {

controller::controller(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("notification", options) {
    m_output_plugins = declare_parameter<std::vector<std::string>>(
        "outputs", {"clover2_notification::outputs::led"});

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
    (void)msg;

    // TODO: output->show(notification_name)
}

}  // namespace clover2_notification
