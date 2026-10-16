#pragma once

#include <clover2_http/base_plugin.hpp>

#include <string>

namespace clover2_http {

template <typename T>
class plugin : public base_plugin {
public:
    std::string name() const override { return std::string(T::k_name); }
    int version() const override { return T::k_version; }
};

}  // namespace clover2_http
