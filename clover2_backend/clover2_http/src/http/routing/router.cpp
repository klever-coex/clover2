#include <clover2_http/http/routing/router.hpp>
#include <clover2_http/http/routing/utils.hpp>

namespace clover2_http::http::routing {

namespace {

std::vector<std::string> path_segments(boost::urls::url_view uv) {
    std::vector<std::string> segs;
    for (const std::string& s : uv.segments()) segs.push_back(s);
    return segs;
}

}  // namespace

void router::add_http_route(boost::beast::http::verb method,
                            const std::string& pattern,
                            std::unique_ptr<endpoint::interface> endpoint) {
    m_http_tries[method].insert(split_pattern(pattern), pattern,
                                std::move(endpoint));

    if (m_logger) {
        m_logger->debug("Registered route {} {}",
                        std::string(boost::beast::http::to_string(method)),
                        pattern);
    }
}

void router::add_ws_route(
    const std::string& pattern,
    std::unique_ptr<transport::ws_handler_interface> handler) {
    m_ws_trie.insert(split_pattern(pattern), pattern, std::move(handler));

    if (m_logger) {
        m_logger->debug("Registered WS route {}", pattern);
    }
}

bool router::dispatch_http(boost::beast::http::verb method,
                           boost::urls::url_view target,
                           core::request_context& ctx,
                           const endpoint::http_request& req,
                           endpoint::response_sender sender) const {
    auto it = m_http_tries.find(method);
    if (it == m_http_tries.end()) {
        if (m_logger) {
            m_logger->debug("No route for {}",
                            std::string(target.encoded_path()));
        }
        return false;
    }

    auto result = it->second.search(path_segments(target));
    if (!result) {
        if (m_logger) {
            m_logger->debug("No route for {}",
                            std::string(target.encoded_path()));
        }
        return false;
    }

    if (m_logger) {
        m_logger->debug("Matched {} for {}",
                        std::string(boost::beast::http::to_string(method)),
                        std::string(target.encoded_path()));
    }
    ctx.path_params = std::move(result->params);
    result->handler->invoke(std::move(ctx), req, std::move(sender));
    return true;
}

transport::ws_handler_interface* router::match_ws(
    boost::urls::url_view target,
    std::unordered_map<std::string, std::string>& path_params) const {
    auto result = m_ws_trie.search(path_segments(target));
    if (!result) {
        if (m_logger) {
            m_logger->debug("No WS route for {}",
                            std::string(target.encoded_path()));
        }
        return nullptr;
    }

    if (m_logger) {
        m_logger->debug("Matched WS for {}",
                        std::string(target.encoded_path()));
    }
    path_params = std::move(result->params);
    return result->handler;
}

}  // namespace clover2_http::http::routing
