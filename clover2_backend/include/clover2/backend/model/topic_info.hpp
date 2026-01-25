#pragma once

#include <clover2/backend/model/base_model.hpp>
#include <nlohmann/json_fwd.hpp>

namespace clover2::backend::model {

class topic_info : public base_model<topic_info> {
public:
    topic_info();
    virtual ~topic_info() = default;

    const std::string& get_id() const;
    void set_id(const std::string& value);

    const std::string& get_name() const;
    void set_name(const std::string& value);

    const std::string& get_type() const;
    void set_type(const std::string& value);

    const nlohmann::json& get_qos_profile() const;
    void set_qos_profile(const nlohmann::json& value);

private:
    std::string m_id;
    std::string m_name;
    std::string m_type;
    nlohmann::json m_qos_profile;
};

void to_json(nlohmann::json& j, const topic_info& p);
void from_json(const nlohmann::json& j, topic_info& p);

}  // namespace clover2::backend::model
