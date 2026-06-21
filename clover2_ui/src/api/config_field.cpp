#include <clover2_ui/api/config_field.hpp>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <iostream>

namespace clover2_ui::api {

config_field::config_field(std::string name, YAML::Node schema,
                           std::shared_ptr<config_field> parent)
    : m_name(std::move(name))
    , m_parent(std::move(parent))
    , m_schema(std::move(schema)) {
    parse_schema();
}

void config_field::parse_schema() {
    if (m_schema["type"]) {
        m_type_str = m_schema["type"].as<std::string>();
    } else {
        m_type_str = "str";
    }

    if (m_schema["description"]) {
        m_description = m_schema["description"].as<std::string>();
    }

    if (m_schema["enum"] && m_schema["enum"].IsSequence()) {
        m_is_enum = true;
        for (const auto& v : m_schema["enum"]) {
            m_enum_values.push_back(v.as<std::string>());
        }
    }

    if (m_type_str.rfind("list", 0) == 0) {
        auto start = m_type_str.find('[');
        auto end = m_type_str.find(']');
        if (start != std::string::npos && end != std::string::npos &&
            end > start + 1) {
            m_list_item_type = m_type_str.substr(start + 1, end - start - 1);
        }
    }
}

std::string config_field::path() const {
    auto p = m_parent.lock();
    if (!p || p->name().empty()) return m_name;
    return p->path() + "." + m_name;
}

bool config_field::has_default() const noexcept {
    return m_schema["default"].IsDefined();
}

YAML::Node config_field::default_value() const { return m_schema["default"]; }

void config_field::reset_to_default() {
    if (has_default()) {
        m_value = YAML::Clone(m_schema["default"]);
    } else {
        m_value.reset();
    }

    m_dirty = true;
}

void config_field::set_value(const YAML::Node& v) {
    m_value = v;
    m_dirty = true;
}

bool config_field::set_enum(const std::string& val) {
    if (!m_is_enum) return false;

    for (const auto& e : m_enum_values) {
        if (e == val) {
            m_value = YAML::Node(val);
            m_dirty = true;
            return true;
        }
    }

    return false;
}

std::shared_ptr<config_field> config_field::child(
    const std::string& name) const {
    for (const auto& c : m_children) {
        if (c->name() == name) return c;
    }

    return nullptr;
}

bool config_field::is_tree_dirty() const {
    if (m_dirty) return true;

    for (const auto& c : m_children) {
        if (c->is_tree_dirty()) return true;
    }

    return false;
}

void config_field::mark_clean() {
    m_dirty = false;

    for (auto& c : m_children) {
        c->mark_clean();
    }
}

YAML::Node config_field::to_yaml_value() const {
    if (is_object()) {
        YAML::Node obj(YAML::NodeType::Map);

        for (const auto& child : m_children) {
            if (!child->name().empty()) {
                obj[child->name()] = child->to_yaml_value();
            }
        }

        return obj;
    }

    if (is_list()) {
        if (m_value.IsDefined() && !m_value.IsNull()) {
            return YAML::Clone(m_value);
        }

        if (has_default()) {
            return YAML::Clone(m_schema["default"]);
        }

        return YAML::Node(YAML::NodeType::Sequence);
    }

    if (m_value.IsDefined() && !m_value.IsNull()) {
        return YAML::Clone(m_value);
    }

    if (has_default()) {
        return YAML::Clone(m_schema["default"]);
    }

    return YAML::Node();
}

void config_field::save_to_file(const std::filesystem::path& file) const {
    std::ofstream out(file);

    if (out) {
        out << to_yaml_value();
    } else {
        throw std::runtime_error("File save failed");
    }
}

bool config_field::is_list() const noexcept {
    return m_type_str.rfind("list", 0) == 0;
}

bool config_field::is_scalar() const noexcept {
    return m_type_str == "str" || m_type_str == "int" ||
           m_type_str == "float" || m_type_str == "bool";
}

YAML::Node config_field::resolve_value(const YAML::Node* values,
                                       const YAML::Node& schema_entry) {
    if (values && *values && !values->IsNull()) {
        return *values;
    }
    if (schema_entry["default"]) {
        return schema_entry["default"];
    }
    return YAML::Node();
}

std::shared_ptr<config_field> config_field::build_tree(
    std::string name, const YAML::Node& schema_entry, const YAML::Node* values,
    std::shared_ptr<config_field> parent) {
    auto field = std::shared_ptr<config_field>(
        new config_field(std::move(name), schema_entry, std::move(parent)));

    if (field->is_object()) {
        field->m_value = YAML::Node(YAML::NodeType::Map);

        if (schema_entry["fields"] && schema_entry["fields"].IsMap()) {
            for (const auto& kv : schema_entry["fields"]) {
                std::string child_name = kv.first.as<std::string>();
                const YAML::Node* child_values = nullptr;
                YAML::Node child_val;
                if (values && *values && values->IsMap() &&
                    (*values)[child_name]) {
                    child_val = (*values)[child_name];
                    child_values = &child_val;
                }
                auto child = build_tree(child_name, kv.second, child_values,
                                        field);
                field->m_children.push_back(std::move(child));
            }
        }
    } else {
        auto val = resolve_value(values, schema_entry);

        if (val.IsMap() && field->is_scalar()) {
            if (schema_entry["default"]) {
                val = schema_entry["default"];
            } else {
                val.reset();
            }
        }

        field->m_value = val;
    }

    return field;
}

std::shared_ptr<config_field> config_field::load_from_nodes(
    const YAML::Node& schema_root, const YAML::Node& values_root) {
    YAML::Node root_schema;
    root_schema["type"] = "object";
    root_schema["description"] = "Root configuration";

    auto root = std::shared_ptr<config_field>(
        new config_field("", root_schema, nullptr));
    root->m_type_str = "object";
    root->m_value = YAML::Node(YAML::NodeType::Map);

    if (schema_root.IsMap()) {
        for (const auto& kv : schema_root) {
            std::string child_name = kv.first.as<std::string>();
            const YAML::Node* child_values = nullptr;
            YAML::Node child_val;
            if (values_root.IsMap() && values_root[child_name]) {
                child_val = values_root[child_name];
                child_values = &child_val;
            }
            auto child =
                build_tree(child_name, kv.second, child_values, root);
            root->m_children.push_back(std::move(child));
        }
    }

    return root;
}

std::shared_ptr<config_field> config_field::load(
    const std::filesystem::path& schema_path,
    const std::filesystem::path& values_path) {
    try {
        YAML::Node schema_root = YAML::LoadFile(schema_path);
        YAML::Node values_root;

        if (!values_path.empty()) {
            try {
                values_root = YAML::LoadFile(values_path);
            } catch (const YAML::Exception& e) {
                std::cerr << "Warning: failed to load config values from "
                          << values_path << ": " << e.what()
                          << ". Using schema defaults." << std::endl;
            }
        }

        return load_from_nodes(schema_root, values_root);
    } catch (const YAML::Exception& e) {
        throw std::runtime_error("Failed to load config from " +
                                 schema_path.string() + ": " + e.what());
    }
}

}  // namespace clover2_ui::api
