#pragma once

#include <clover2_http/server.hpp>

#include <memory>

namespace clover2_http_bridge {

class base_plugin {
public:
    using SharedPtr = std::shared_ptr<base_plugin>;
    virtual ~base_plugin() = default;

    virtual void initialize(std::shared_ptr<clover2_http::server> server) = 0;
};

}  // namespace clover2_http_bridge
