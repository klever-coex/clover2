#include <clover2_notification/controller.hpp>
#include <clover2_notification/provider/factory.hpp>

#include <exception>
#include <functional>
#include <stdexcept>

namespace clover2_notification {

controller::controller(const rclcpp::NodeOptions& options)
    : clover2_common::node("notification", options) {
    m_provider_names = declare_parameter<std::vector<std::string>>(
        "providers_list", {"diagnostics"});
    m_output_ids = declare_parameter<std::vector<std::string>>("output_plugins",
                                                               {"led_strip"});
    declare_parameter<std::string>("led_strip.plugin", "led");

    try {
        m_node_context = std::make_shared<clover2_common::node_context>(*this);

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
            const auto enabled_param = output_id + ".enabled";
            if (!has_parameter(enabled_param)) {
                declare_parameter<bool>(enabled_param, true);
            }

            bool enabled = true;
            if (!get_parameter(enabled_param, enabled) || !enabled) {
                RCLCPP_INFO(get_logger(),
                            "Notification output disabled: id='%s'",
                            output_id.c_str());
                continue;
            }

            const auto plugin_param = output_id + ".plugin";
            if (!has_parameter(plugin_param)) {
                declare_parameter<std::string>(plugin_param);
            }

            std::string plugin_name;
            if (!get_parameter(plugin_param, plugin_name) ||
                plugin_name.empty()) {
                throw std::runtime_error(
                    "Notification output plugin is not set: " + plugin_param);
            }

            auto plugin = m_output_loader.createSharedInstance(plugin_name);
            plugin->initialize(m_node_context, output_id);
            m_outputs.emplace_back(std::move(plugin));
            RCLCPP_INFO(get_logger(),
                        "Loaded notification output: id='%s' plugin='%s'",
                        output_id.c_str(), plugin_name.c_str());
        }
    } catch (const std::exception& e) {
        RCLCPP_ERROR(get_logger(), "Failed to initialize notification: %s",
                     e.what());
        for (const auto& provider : m_providers) {
            provider->cleanup();
        }
        for (const auto& output : m_outputs) {
            output->clear();
        }
        m_providers.clear();
        m_outputs.clear();
        m_node_context.reset();
    }
}

controller::~controller() {
    for (const auto& provider : m_providers) {
        provider->cleanup();
    }
    for (const auto& output : m_outputs) {
        output->clear();
    }
    m_providers.clear();
    m_outputs.clear();
    m_node_context.reset();
}

void controller::provider_callback(const data::event& event) {
    for (const auto& output : m_outputs) {
        output->push2queue(event);
    }
}

}  // namespace clover2_notification

#include <rclcpp_components/register_node_macro.hpp>

RCLCPP_COMPONENTS_REGISTER_NODE(clover2_notification::controller)
