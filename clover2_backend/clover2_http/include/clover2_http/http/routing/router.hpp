#pragma once

#include <clover2_http/http/core/logger.hpp>
#include <clover2_http/http/endpoint/interface.hpp>
#include <clover2_http/http/routing/trie.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/url.hpp>

#include <memory>
#include <string>
#include <unordered_map>

namespace clover2_http::http::routing {

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
                       boost::urls::url_view target, core::request_context& ctx,
                       const endpoint::http_request& req,
                       endpoint::response_sender sender) const;

    transport::ws_handler_interface* match_ws(
        boost::urls::url_view target,
        std::unordered_map<std::string, std::string>& path_params) const;

private:
    std::shared_ptr<clover2_http::http::core::logger> m_logger;
    std::unordered_map<boost::beast::http::verb, trie<endpoint::interface>>
        m_http_tries;
    trie<transport::ws_handler_interface> m_ws_trie;
};

}  // namespace clover2_http::http::routing
