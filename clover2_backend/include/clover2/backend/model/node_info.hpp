#pragma once

#include <clover2/backend/model/base_model.hpp>
#include <clover2/backend/model/data/node_type.hpp>
#include <nlohmann/json_fwd.hpp>
#include <vector>

namespace clover2::backend::model {
struct node_info : public base_model<node_info> {
    node_info();
    virtual ~node_info() = default;

    const std::string& get_id() const;
    void set_id(const std::string& value);

    const std::string& get_name() const;
    void set_name(const std::string& value);

    const std::string& get_namespace() const;
    void set_namespace(const std::string& value);

    data::node_type get_type() const;
    void set_type(const data::node_type& value);

    const std::vector<std::string>& get_publishing_topics() const;
    void set_publishing_topics(const std::vector<std::string>& value);

    const std::vector<std::string>& get_subscribing_topics() const;
    void set_subscribing_topics(const std::vector<std::string>& value);

private:
    std::string m_id;
    std::string m_name;
    std::string m_namespace;
    data::node_type m_type;
    std::vector<std::string> m_publishing_topics;
    std::vector<std::string> m_subscribing_topics;
};

void to_json(nlohmann::json& j, const node_info& p);
void from_json(const nlohmann::json& j, node_info& p);

}  // namespace clover2::backend::model
