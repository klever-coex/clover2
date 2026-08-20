#pragma once

// clover2
#include <clover2_http/http/core/request_context.hpp>
#include <clover2_http/http/endpoint/reply.hpp>

// STL
#include <functional>

namespace clover2_http::http::middleware {

class base_middleware {
public:
    using next_t = std::function<void(core::request_context&,
                                      http::endpoint::http_request&,
                                      http::endpoint::reply_base&)>;

    virtual ~base_middleware() = default;

    virtual void handle(core::request_context& ctx,
                        http::endpoint::http_request& req,
                        http::endpoint::reply_base& reply, next_t next) = 0;
};

}  // namespace clover2_http::http::middleware
