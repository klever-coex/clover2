#pragma once

#include <clover2/backend/model/base_model.hpp>
#include <clover2/backend/model/node_info.hpp>

namespace clover2::backend::model {

struct nodes : public base_model<nodes> {
public:
    nodes();
    virtual ~nodes() = default;

    const std::vector<node_info>& get_nodes() const;
    void set_nodes(const std::vector<node_info>& nodes);

private:
    std::vector<node_info> m_nodes;
};

void to_json(nlohmann::json& j, const nodes& p);
void from_json(const nlohmann::json& j, nodes& p);

}  // namespace clover2::backend::model
