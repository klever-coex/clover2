#pragma once

// clover2
#include <clover2_http/http/core/logger.hpp>
#include <clover2_http/http/endpoint/interface.hpp>
#include <clover2_http/http/middleware/base_middleware.hpp>
#include <clover2_http/http/routing/trie.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

// boost
#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/url.hpp>

// STL
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_http::http::routing {

class router {
public:
    using middleware_creator =
        std::function<std::unique_ptr<middleware::base_middleware>()>;

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

    void add_middleware(const std::string& pattern, middleware_creator creator);

    bool dispatch_http(boost::beast::http::verb method,
                       boost::urls::url_view target, core::request_context& ctx,
                       endpoint::http_request req,
                       endpoint::response_sender sender) const;

    transport::ws_handler_interface* match_ws(
        boost::urls::url_view target,
        std::unordered_map<std::string, std::string>& path_params) const;

private:
    std::shared_ptr<clover2_http::http::core::logger> m_logger;

    size_t m_max_mw_depth = 0;
    trie<std::vector<middleware_creator>> m_middleware_trie;
    trie<transport::ws_handler_interface> m_ws_trie;
    std::unordered_map<boost::beast::http::verb, trie<endpoint::interface>>
        m_http_tries;
};

}  // namespace clover2_http::http::routing
