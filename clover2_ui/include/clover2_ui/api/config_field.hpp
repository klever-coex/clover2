#pragma once

#include <yaml-cpp/node/node.h>
#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace clover2_ui::api {

class config_field : public std::enable_shared_from_this<config_field> {
public:
    const std::string& name() const noexcept { return m_name; }
    std::shared_ptr<config_field> parent() const { return m_parent.lock(); }
    std::string path() const;

    const std::string& type_str() const noexcept { return m_type_str; }
    const std::string& description() const noexcept { return m_description; }

    bool is_object() const noexcept { return m_type_str == "object"; }
    bool is_list() const noexcept;
    bool is_scalar() const noexcept;

    bool has_default() const noexcept;
    YAML::Node default_value() const;
    void reset_to_default();

    bool is_enum() const noexcept { return m_is_enum; }
    const std::vector<std::string>& enum_values() const noexcept {
        return m_enum_values;
    }

    const YAML::Node& value() const noexcept { return m_value; }
    void set_value(const YAML::Node& v);

    template <typename T>
    T as() const {
        try {
            if (m_value.IsDefined() && !m_value.IsNull()) {
                return m_value.as<T>();
            }
            if (has_default()) return default_value().as<T>();
        } catch (const YAML::BadConversion&) {
            // Type mismatch — fall through to default
        }
        return T{};
    }

    template <typename T>
    void set(T val) {
        m_value = YAML::Node(val);
        m_dirty = true;
    }

    bool set_enum(const std::string& val);

    const std::vector<std::shared_ptr<config_field>>& children()
        const noexcept {
        return m_children;
    }
    std::shared_ptr<config_field> child(const std::string& name) const;

    bool is_dirty() const noexcept { return m_dirty; }
    bool is_tree_dirty() const;
    void mark_clean();

    YAML::Node to_yaml_value() const;
    void save_to_file(const std::filesystem::path& file) const;

    static std::shared_ptr<config_field> load(
        const std::filesystem::path& schema_path,
        const std::filesystem::path& values_path = "");

    static std::shared_ptr<config_field> load_from_nodes(
        const YAML::Node& schema_root, const YAML::Node& values_root);

private:
    config_field(std::string name, YAML::Node schema,
                 std::shared_ptr<config_field> parent);

    static std::shared_ptr<config_field> build_tree(
        std::string name, const YAML::Node& schema_entry,
        const YAML::Node* values,
        std::shared_ptr<config_field> parent);

    void parse_schema();

    static YAML::Node resolve_value(const YAML::Node* values,
                                    const YAML::Node& schema_entry);

    std::string m_name;
    std::weak_ptr<config_field> m_parent;

    YAML::Node m_schema;
    std::string m_type_str;
    std::string m_description;
    bool m_is_enum = false;
    std::vector<std::string> m_enum_values;
    std::string m_list_item_type;

    YAML::Node m_value;
    bool m_dirty = false;

    std::vector<std::shared_ptr<config_field>> m_children;
};

}  // namespace clover2_ui::api
