#pragma once

#include <clover2/backend/model/base_model.hpp>
#include <nlohmann/json_fwd.hpp>

namespace clover2::backend::model {

class action_info : public base_model<action_info> {
public:
    action_info();
    virtual ~action_info() = default;

    const std::string& get_id() const;
    void set_id(const std::string& value);

    const std::string& get_name() const;
    void set_name(const std::string& value);

    const std::string& get_type() const;
    void set_type(const std::string& value);

private:
    std::string m_id;
    std::string m_name;
    std::string m_type;
};

void to_json(nlohmann::json& j, const action_info& p);
void from_json(const nlohmann::json& j, action_info& p);

}  // namespace clover2::backend::model

