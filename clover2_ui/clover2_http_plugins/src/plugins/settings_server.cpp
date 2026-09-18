// clover2
#include <clover2_common/util/parameter.hpp>
#include <clover2_http/plugin.hpp>
#include <clover2_http_plugins/data/modify_result.hpp>
#include <clover2_http_plugins/data/settings_schema.hpp>
#include <clover2_http_plugins/data/settings_values.hpp>
#include <clover2_ui/api/settings/config_field.hpp>

// ROS2
#include <rclcpp/rclcpp.hpp>

// YAML / JSON
#include <nlohmann/json.hpp>

// STL
#include <filesystem>
#include <format>
#include <string>
#include <utility>
#include <vector>

namespace clover2_http_plugins {

namespace http = clover2_http::http;
using clover2_http_plugins::data::modify_result;
using clover2_http_plugins::data::settings_schema;
using clover2_http_plugins::data::settings_values;
using clover2_ui::api::settings::config_field;
using clover2_ui::api::settings::field_type;

class settings_server : public clover2_http::plugin<settings_server> {
public:
    static constexpr std::string_view k_name = "settings_server";
    static constexpr int k_version = 1;

protected:
    void on_initialize() override;
    std::vector<std::string> capabilities() const override;

private:
    void handle_schema(http::core::request_context /*ctx*/,
                       http::endpoint::deferred_reply<settings_schema> reply);

    void handle_save(http::core::request_context /*ctx*/,
                     settings_values request,
                     http::endpoint::deferred_reply<modify_result> reply);

    std::shared_ptr<config_field> load_root() const;

    std::filesystem::path m_schema_path;
    std::filesystem::path m_values_path;
};

namespace {

nlohmann::json yaml_scalar_to_json(const config_field& field,
                                   const YAML::Node& value) {
    try {
        switch (field.type()) {
            case field_type::BOOL:
                return value.as<bool>();
            case field_type::INT:
                return static_cast<int64_t>(value.as<int64_t>());
            case field_type::FLOAT:
                return value.as<double>();
            case field_type::STR:
                return value.as<std::string>();
            default:
                return nullptr;
        }
    } catch (const YAML::Exception&) {
        return nullptr;
    }
}

nlohmann::json effective_value_to_json(const config_field& field) {
    const auto& value = field.value();
    if (value.IsDefined() && !value.IsNull()) {
        return yaml_scalar_to_json(field, value);
    }

    if (field.has_default()) {
        return yaml_scalar_to_json(field, field.default_value());
    }

    return nullptr;
}

nlohmann::json to_schema_json(const config_field& field) {
    nlohmann::json j;

    j["name"] = field.name();
    j["type"] = field.type_str();

    if (!field.description().empty()) {
        j["description"] = field.description();
    }

    if (field.is_object()) {
        nlohmann::json children = nlohmann::json::array();
        for (const auto& child : field.children()) {
            children.push_back(to_schema_json(*child));
        }
        j["children"] = std::move(children);
        return j;
    }

    if (field.is_enum()) {
        j["enum"] = field.enum_values();
    }

    if (field.has_default()) {
        j["default"] = yaml_scalar_to_json(field, field.default_value());
    }

    j["value"] = effective_value_to_json(field);

    return j;
}

std::string json_scalar_to_text(const nlohmann::json& node) {
    switch (node.type()) {
        case nlohmann::json::value_t::boolean:
            return node.get<bool>() ? "true" : "false";
        case nlohmann::json::value_t::number_integer:
        case nlohmann::json::value_t::number_unsigned:
            return std::to_string(node.get<int64_t>());
        case nlohmann::json::value_t::number_float:
            return std::format("{}", node.get<double>());
        case nlohmann::json::value_t::string:
            return node.get<std::string>();
        default:
            return node.dump();
    }
}

void set_scalar(config_field& field, const nlohmann::json& node,
                const std::string& path) {
    const auto fail = [&](const char* what) {
        throw http::core::http_error(400, path + ": " + what);
    };

    if (node.is_null()) {
        fail("expected a value");
    }

    if (field.is_enum()) {
        if (!node.is_string()) fail("expected a string");
    } else {
        switch (field.type()) {
            case field_type::BOOL:
                if (!node.is_boolean()) fail("expected a boolean");
                break;
            case field_type::INT:
                if (!node.is_number_integer()) fail("expected an integer");
                break;
            case field_type::FLOAT:
                if (!node.is_number()) fail("expected a number");
                break;
            case field_type::STR:
                if (!node.is_string()) fail("expected a string");
                break;
            default:
                break;
        }
    }

    try {
        field.set_from_string(json_scalar_to_text(node));
    } catch (const std::invalid_argument& e) {
        fail(e.what());
    }
}

void apply_values(config_field& field, const nlohmann::json& node,
                  const std::string& path) {
    if (field.is_object()) {
        if (!node.is_object()) {
            throw http::core::http_error(400, path + ": expected an object");
        }

        for (const auto& child : field.children()) {
            const std::string& name = child->name();
            if (node.contains(name)) {
                apply_values(*child, node.at(name),
                             path.empty() ? name : path + "." + name);
            } else {
                child->reset_to_default();
            }
        }

        for (auto it = node.begin(); it != node.end(); ++it) {
            if (field.child(it.key()) == nullptr) {
                throw http::core::http_error(
                    400, path + ": unknown key \"" + it.key() + "\"");
            }
        }

        return;
    }

    set_scalar(field, node, path);
}

}  // namespace

void settings_server::on_initialize() {
    auto parameters = m_node_context->get_node_parameters_interface();

    clover2_common::util::declare_parameter_if_not_declared(
        parameters, "settings_server.schema_path", "");
    clover2_common::util::declare_parameter_if_not_declared(
        parameters, "settings_server.values_path", "");

    rclcpp::Parameter schema_param;
    rclcpp::Parameter values_param;
    parameters->get_parameter("settings_server.schema_path", schema_param);
    parameters->get_parameter("settings_server.values_path", values_param);

    m_schema_path = schema_param.as_string();
    m_values_path = values_param.as_string();

    if (m_schema_path.empty()) {
        RCLCPP_WARN(get_logger(),
                    "settings_server.schema_path is not set - the settings API "
                    "is disabled");
    }

    m_server->get<settings_schema>(
        "/api/settings/schema",
        std::bind(&settings_server::handle_schema, this, std::placeholders::_1,
                  std::placeholders::_2));

    m_server->put<settings_values, modify_result>(
        "/api/settings/values",
        std::bind(&settings_server::handle_save, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
}

std::vector<std::string> settings_server::capabilities() const {
    if (m_schema_path.empty() || !std::filesystem::exists(m_schema_path)) {
        return {};
    }

    return {"settings"};
}

std::shared_ptr<config_field> settings_server::load_root() const {
    if (m_schema_path.empty()) {
        throw http::core::http_error(503,
                                     "Settings schema path is not configured");
    }

    try {
        return config_field::load(m_schema_path, m_values_path);
    } catch (const std::exception& e) {
        throw http::core::http_error(503, e.what());
    }
}

void settings_server::handle_schema(
    http::core::request_context /*ctx*/,
    http::endpoint::deferred_reply<settings_schema> reply) {
    try {
        const auto root = load_root();
        reply(settings_schema{true, to_schema_json(*root)}, 200);
    } catch (const http::core::http_error& e) {
        reply(settings_schema{false, nullptr}, e.status());
    }
}

void settings_server::handle_save(
    http::core::request_context /*ctx*/, settings_values request,
    http::endpoint::deferred_reply<modify_result> reply) {
    if (m_values_path.empty()) {
        reply(modify_result{false, "Settings values path is not configured"},
              503);
        return;
    }

    std::shared_ptr<config_field> root;
    try {
        root = load_root();
        apply_values(*root, request.values, "");
    } catch (const http::core::http_error& e) {
        reply(modify_result{false, e.message()}, e.status());
        return;
    }

    try {
        root->save_to_file(m_values_path);
    } catch (const std::exception& e) {
        reply(modify_result{false, e.what()}, 500);
        return;
    }

    root->mark_clean();
    reply(modify_result{true, ""}, 200);
}

}  // namespace clover2_http_plugins

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_http_plugins::settings_server,
                       clover2_http::base_plugin)
