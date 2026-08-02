#pragma once

#include <clover2_http/http/core/logger.hpp>
#include <clover2_http/http/endpoint/interface.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/http/verb.hpp>

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_http::http::routing {

struct parsed_pattern {
    std::vector<std::string> segments;
    std::vector<uint8_t> is_param;
    std::vector<std::string> param_names;
};

class router {
public:
    router() = default;

    explicit router(std::shared_ptr<clover2_http::http::core::logger> log)
        : m_logger(std::move(log)) {}

    router(const router&) = delete;
    router& operator=(const router&) = delete;

    void add_http_route(boost::beast::http::verb method,
                        const std::string& pattern,
                        std::unique_ptr<endpoint::interface> endpoint);

    void add_ws_route(const std::string& pattern,
                      std::unique_ptr<transport::ws_handler_interface> handler);

    bool dispatch_http(boost::beast::http::verb method,
                       const std::string& target, core::request_context& ctx,
                       const endpoint::http_request& req,
                       endpoint::response_sender sender) const;

    transport::ws_handler_interface* match_ws(
        const std::string& target,
        std::unordered_map<std::string, std::string>& path_params) const;

private:
    struct route_entry {
        boost::beast::http::verb method;
        std::string pattern;
        parsed_pattern parsed;
        std::unique_ptr<endpoint::interface> endpoint;
    };

    struct ws_route_entry {
        std::string pattern;
        parsed_pattern parsed;
        std::unique_ptr<transport::ws_handler_interface> handler;
    };

    std::shared_ptr<clover2_http::http::core::logger> m_logger;
    std::vector<route_entry> m_http_routes;
    std::vector<ws_route_entry> m_ws_routes;
};

}  // namespace clover2_http::http::routing
