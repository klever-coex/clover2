#pragma once

#include <clover2_http/http/core/exceptions.hpp>
#include <clover2_http/http/core/request_context.hpp>
#include <clover2_http/http/endpoint/reply.hpp>

namespace clover2_http::http::endpoint {

class interface {
public:
    virtual ~interface() = default;
    virtual void invoke(core::request_context& ctx, http_request& req,
                        reply_base& reply) = 0;
};

}  // namespace clover2_http::http::endpoint
