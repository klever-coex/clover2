#pragma once

#include <stdexcept>
#include <string>

namespace clover2::backend::model::data {

class node_type {
public:
    enum class type {
        simple_node,
        lifecycle_node,
    };

    node_type()
        : m_type(type::simple_node) {}
    node_type(const type& t)
        : m_type(t) {}
    virtual ~node_type() = default;

    operator type() const { return m_type; }

    static std::string to_string(const type& nodeType) {
        switch (nodeType) {
            case type::simple_node:
                return "node";
            case type::lifecycle_node:
                return "lifecycle";
            default:
                return "unknown";
        }
    }

    static node_type from_string(const std::string& str) {
        if (str == "node") {
            return {type::simple_node};
        } else if (str == "lifecycle") {
            return {type::lifecycle_node};
        } else {
            throw std::invalid_argument("Invalid node type string: " + str);
        }
    }

private:
    type m_type;
};

}  // namespace clover2::backend::model::data
