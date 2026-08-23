// clover2
#include <clover2_http/http/routing/router.hpp>
#include <clover2_http/http/routing/utils.hpp>

// STL
#include <algorithm>
#include <span>

namespace clover2_http::http::routing {

namespace {

std::vector<std::string> path_segments(boost::urls::url_view uv) {
    std::vector<std::string> segs;
    for (const std::string& s : uv.segments()) segs.push_back(s);
    return segs;
}

using middleware_chain =
    std::vector<std::unique_ptr<middleware::base_middleware>>;
using middleware_terminal = std::function<void(
    core::request_context&, endpoint::http_request&, endpoint::reply_base&)>;

void run_middleware(middleware_chain& chain, size_t i,
                    core::request_context& ctx, endpoint::http_request& req,
                    endpoint::reply_base& reply,
                    const middleware_terminal& terminal) {
    if (i == chain.size()) {
        terminal(ctx, req, reply);
        return;
    }

    middleware::base_middleware::next_t next =
        [&](core::request_context& ctx, endpoint::http_request& req,
            endpoint::reply_base& reply) {
            run_middleware(chain, i + 1, ctx, req, reply, terminal);
        };

    chain[i]->handle(ctx, req, reply, next);
}

}  // namespace

void router::add_http_route(boost::beast::http::verb method,
                            const std::string& pattern,
                            std::unique_ptr<endpoint::interface> endpoint) {
    auto [_, inserted] = m_http_tries[method].insert(
        split_pattern(pattern), pattern, std::move(endpoint));

    if (!inserted) {
        throw core::routing_error("Route {} already exists", pattern);
    }

    if (m_logger) {
        m_logger->debug("Registered route {} {}",
                        std::string(boost::beast::http::to_string(method)),
                        pattern);
    }
}

void router::add_ws_route(
    const std::string& pattern,
    std::unique_ptr<transport::ws_handler_interface> handler) {
    auto [_, inserted] =
        m_ws_trie.insert(split_pattern(pattern), pattern, std::move(handler));

    if (!inserted) {
        throw core::routing_error("Route {} already exists", pattern);
    }

    if (m_logger) {
        m_logger->debug("Registered WS route {}", pattern);
    }
}

void router::add_middleware(const std::string& pattern,
                            middleware_creator creator) {
    auto segs = split_pattern(pattern);

    if (std::any_of(segs.begin(), segs.end(), is_catch_all)) {
        throw core::routing_error(
            "Catch-all is not allowed in middleware pattern: {}", pattern);
    }

    auto [handlers, _] = m_middleware_trie.insert(
        segs, pattern, std::make_unique<std::vector<middleware_creator>>());
    handlers->push_back(std::move(creator));
    m_max_mw_depth = std::max(m_max_mw_depth, segs.size());

    if (m_logger) {
        m_logger->debug("Registered middleware {}", pattern);
    }
}

bool router::dispatch_http(boost::beast::http::verb method,
                           boost::urls::url_view target,
                           core::request_context& ctx,
                           endpoint::http_request req,
                           endpoint::response_sender sender) const {
    auto segs = path_segments(target);

    middleware_chain chain;
    chain.reserve(m_max_mw_depth);
    for (size_t len = 0; len <= std::min(segs.size(), m_max_mw_depth); ++len) {
        auto result = m_middleware_trie.search(std::span(segs).first(len));
        if (!result) {
            continue;
        }

        for (const auto& create : *result->handler) {
            chain.push_back(create());
        }
    }

    bool found = false;

    // The reply lives on the heap: the endpoint may complete it after
    // the dispatch returns (deferred replies); the last shared copy
    // dropping unsent sends the 500 fallback.
    auto reply = std::make_shared<endpoint::reply_base>(std::move(sender));

    middleware_terminal terminal = [this, method, target, &segs, &found,
                                    reply](core::request_context& ctx,
                                           endpoint::http_request& req,
                                           endpoint::reply_base& reply_ref) {
        auto it = m_http_tries.find(method);
        if (it == m_http_tries.end()) {
            if (m_logger) {
                m_logger->debug("No route for {}",
                                std::string(target.encoded_path()));
            }

            reply_ref.error_json(404, "Not Found");
            return;
        }

        auto result = it->second.search(segs);
        if (!result) {
            if (m_logger) {
                m_logger->debug("No route for {}",
                                std::string(target.encoded_path()));
            }

            reply_ref.error_json(404, "Not Found");
            return;
        }

        if (m_logger) {
            m_logger->debug("Matched {} for {}",
                            std::string(boost::beast::http::to_string(method)),
                            std::string(target.encoded_path()));
        }

        found = true;
        ctx.path_params = std::move(result->params);
        result->handler->invoke(ctx, req, reply);
    };

    try {
        run_middleware(chain, 0, ctx, req, *reply, terminal);
    } catch (const core::http_error& e) {
        reply->error_json(e.status(), e.message());
    } catch (const std::exception& e) {
        if (m_logger) {
            m_logger->error("Exception in request handling: {}", e.what());
        }

        reply->error_json(500, "Internal Server Error");
    }

    return found;
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
