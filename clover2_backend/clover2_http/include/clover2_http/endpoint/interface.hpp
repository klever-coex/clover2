#pragma once

#include <clover2_http/core/error.hpp>
#include <clover2_http/core/request_context.hpp>
#include <clover2_http/endpoint/reply.hpp>

namespace clover2_http::endpoint {

class interface {
public:
    virtual ~interface() = default;
    virtual void invoke(core::request_context ctx, http_request req,
                        response_sender sender) = 0;
};

}  // namespace clover2_http::endpoint
