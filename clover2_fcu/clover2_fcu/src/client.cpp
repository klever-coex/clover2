/**
 * @file client.cpp
 * @brief Provides the FCU interface client implementation.
 */

// clover2
#include <clover2_common/util/sync_service.hpp>
#include <clover2_fcu/client.hpp>

// STL
#include <algorithm>

namespace clover2_fcu {

using clover2_common::util::call_sync;

void client::connect() {
    const std::string service_name = m_settings.bridge_node() + "/get_config";

    if (!m_config_client->wait_for_service(m_settings.wait_timeout())) {
        throw exception("fcu bridge is not available: {}", service_name);
    }

    std::shared_ptr<clover2_fcu_msgs::srv::GetConfig::Response> response;
    try {
        response = call_sync<clover2_fcu_msgs::srv::GetConfig>(
            m_config_client, service_name,
            std::make_shared<clover2_fcu_msgs::srv::GetConfig::Request>(),
            m_settings.wait_timeout());
    } catch (const std::runtime_error& e) {
        throw exception("fcu bridge error: {}", e.what());
    }

    if (!response->ok) {
        throw exception("fcu bridge rejected the configuration request: {}",
                        response->error);
    }

    apply_config(response->config);
}

std::shared_ptr<void> client::get(const std::string& capability) {
    return backend()->get(capability);
}

void client::apply_config(const clover2_fcu_msgs::msg::FcuConfig& config) {
    if (!config.plugin_params.empty()) {
        try {
            m_params = YAML::Load(config.plugin_params);
        } catch (const YAML::Exception& e) {
            throw exception("invalid plugin params yaml: {}", e.what());
        }

        if (!m_params.IsNull() && !m_params.IsMap()) {
            throw exception(
                "plugin params yaml must be a map keyed by capability name");
        }
    }

    m_config = config;

    // Publish the client config into the runtime environment (providers
    // read their subtree via base_interface::params(name)).
    m_base->params(m_params);

    check_requirements();

    RCLCPP_INFO(m_base->logger(), "fcu client connected: protocol='%s'",
                m_config.protocol.c_str());
}

void client::check_requirements() {
    std::string missing;
    for (const std::string& capability_name : m_settings.required()) {
        if (!has(capability_name)) {
            if (!missing.empty()) {
                missing += ", ";
            }

            missing += capability_name;
        }
    }

    if (!missing.empty()) {
        throw exception(
            "required capabilities not served by the resolved factories: {}",
            missing);
    }
}

bool client::has(const std::string& capability) {
    try {
        const auto capabilities = backend()->capabilities();
        return std::find(capabilities.begin(), capabilities.end(),
                         capability) != capabilities.end();
    } catch (const exception&) {
        return false;
    }
}

backend::backend_base::shared_ptr client::backend() {
    auto backend_instance = ensure_backend(m_config.protocol);
    backend_instance->initialize(m_base);

    return backend_instance;
}

backend::backend_base::shared_ptr client::ensure_backend(
    const std::string& plugin_name) {
    const auto existing = m_backends.find(plugin_name);
    if (existing != m_backends.end()) {
        return existing->second;
    }

    try {
        auto instance = m_loader.createSharedInstance(plugin_name);
        m_backends[plugin_name] = instance;

        return instance;
    } catch (const pluginlib::PluginlibException& e) {
        throw exception(
            "failed to load the factory '{}' (is the backend package in the "
            "workspace?): {}",
            plugin_name, e.what());
    }
}

}  // namespace clover2_fcu
