#pragma once

// clover2
#include <clover2_http/http/middleware/base_middleware.hpp>

// boost
#include <boost/beast/http/verb.hpp>

namespace clover2_http::http::middleware {

class cors : public base_middleware {
public:
    void handle(core::request_context& ctx, http::endpoint::http_request& req,
                http::endpoint::reply_base& reply, next_t next) override {
        reply.header("Access-Control-Allow-Origin", "*");
        reply.header("Access-Control-Allow-Methods",
                         "GET, POST, PUT, DELETE, PATCH, OPTIONS");
        reply.header("Access-Control-Allow-Headers", "Content-Type");
        reply.header("Access-Control-Max-Age", "86400");

        if (req.method() == boost::beast::http::verb::options &&
            req.find(
                boost::beast::http::field::access_control_request_method) !=
                req.end()) {
            reply.error(204);
            return;
        }

        next(ctx, req, reply);
    }
};

}  // namespace clover2_http::http::middleware
