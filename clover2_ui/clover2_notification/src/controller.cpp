#include <clover2_notification/controller.hpp>
#include <clover2_notification/provider/factory.hpp>

#include <exception>
#include <functional>
#include <stdexcept>

namespace clover2_notification {

controller::controller(const rclcpp::NodeOptions& options)
    : clover2_common::lifecycle_node("notification", options) {
    m_provider_names = declare_parameter<std::vector<std::string>>(
        "providers", {"diagnostics"});
    m_output_ids = declare_parameter<std::vector<std::string>>(
        "output_plugins", {"led_strip"});
    declare_parameter<std::string>("led_strip.plugin", "led");

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

        for (const auto& provider_name : m_provider_names) {
            auto provider = provider::factory::instance().create(provider_name);
            if (!provider) {
                throw std::runtime_error("Unknown notification provider: " +
                                         provider_name);
            }

            provider->initialize(m_node_context,
                                 std::bind(&controller::provider_callback, this,
                                           std::placeholders::_1));
            m_providers.emplace_back(std::move(provider));
            RCLCPP_INFO(get_logger(), "Loaded notification provider: %s",
                        provider_name.c_str());
        }

        for (const auto& output_id : m_output_ids) {
            const auto plugin_param = output_id + ".plugin";
            if (!has_parameter(plugin_param)) {
                declare_parameter<std::string>(plugin_param);
            }

            std::string plugin_name;
            if (!get_parameter(plugin_param, plugin_name) || plugin_name.empty()) {
                throw std::runtime_error("Notification output plugin is not set: " +
                                         plugin_param);
            }

            auto plugin = m_output_loader.createSharedInstance(plugin_name);
            plugin->initialize(node, output_id);
            m_outputs.emplace_back(std::move(plugin));
            RCLCPP_INFO(get_logger(),
                        "Loaded notification output: id='%s' plugin='%s'",
                        output_id.c_str(), plugin_name.c_str());
        }
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Failed to configure notification: %s",
                     e.what());
        for (const auto& provider : m_providers) {
            provider->cleanup();
        }
        m_providers.clear();
        m_outputs.clear();
        m_node_context.reset();
        return CallbackReturn::FAILURE;
    }

    return CallbackReturn::SUCCESS;
}

controller::CallbackReturn controller::on_activate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    return CallbackReturn::SUCCESS;
}

controller::CallbackReturn controller::on_deactivate(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    for (const auto& output : m_outputs) {
        output->clear();
    }
    return CallbackReturn::SUCCESS;
}

controller::CallbackReturn controller::on_cleanup(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    for (const auto& provider : m_providers) {
        provider->cleanup();
    }
    for (const auto& output : m_outputs) {
        output->clear();
    }
    m_providers.clear();
    m_outputs.clear();
    m_node_context.reset();
    return CallbackReturn::SUCCESS;
}

controller::CallbackReturn controller::on_shutdown(
    [[maybe_unused]] const rclcpp_lifecycle::State& state) {
    for (const auto& provider : m_providers) {
        provider->cleanup();
    }
    for (const auto& output : m_outputs) {
        output->clear();
    }
    m_providers.clear();
    m_outputs.clear();
    m_node_context.reset();
    return CallbackReturn::SUCCESS;
}

void controller::provider_callback(const data::event& event) {
    for (const auto& output : m_outputs) {
        output->push2queue(event);
    }
}

}  // namespace clover2_notification
