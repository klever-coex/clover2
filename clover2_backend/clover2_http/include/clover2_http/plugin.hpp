#pragma once

#include <clover2_http/base_plugin.hpp>

#include <string>

namespace clover2_http {

template <typename Derived>
class plugin : public base_plugin {
public:
    std::string name() const override { return std::string(Derived::k_name); }
    int version() const override { return Derived::k_version; }
};

}  // namespace clover2_http
