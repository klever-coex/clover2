#include <clover2_http/http/routing/router.hpp>

#include <cassert>
#include <string_view>

namespace clover2_http::http::routing {

namespace {

std::vector<std::string> tokenize_owned(std::string_view path) {
    std::vector<std::string> segs;
    if (path.empty() || path == "/") return segs;

    size_t start = (path[0] == '/') ? 1 : 0;
    while (start < path.size()) {
        size_t end = path.find('/', start);

        if (end == std::string_view::npos) {
            segs.emplace_back(path.substr(start));
            break;
        }

        if (end > start) segs.emplace_back(path.substr(start, end - start));
        start = end + 1;
    }

    return segs;
}

std::vector<std::string_view> tokenize_sv(std::string_view path) {
    std::vector<std::string_view> segs;
    if (path.empty() || path == "/") return segs;

    size_t start = (path[0] == '/') ? 1 : 0;

    while (start < path.size()) {
        size_t end = path.find('/', start);

        if (end == std::string_view::npos) {
            segs.push_back(path.substr(start));
            break;
        }

        if (end > start) segs.push_back(path.substr(start, end - start));
        start = end + 1;
    }

    return segs;
}

std::string_view strip_query(std::string_view target) {
    auto pos = target.find('?');
    return (pos != std::string_view::npos) ? target.substr(0, pos) : target;
}

parsed_pattern parse_pattern(const std::string& pattern) {
    parsed_pattern pp;
    pp.segments = tokenize_owned(pattern);
    pp.is_param.resize(pp.segments.size(), 0);

    for (size_t i = 0; i < pp.segments.size(); ++i) {
        const auto& seg = pp.segments[i];
        if (seg.size() > 2 && seg.front() == '{' && seg.back() == '}') {
            pp.is_param[i] = 1;
            pp.param_names.push_back(seg.substr(1, seg.size() - 2));
        }
    }
    return pp;
}

bool match_segments(const std::vector<std::string_view>& path_segs,
                    const parsed_pattern& parsed,
                    std::unordered_map<std::string, std::string>& params) {
    assert(path_segs.size() == parsed.segments.size());

    size_t param_idx = 0;
    for (size_t i = 0; i < parsed.segments.size(); ++i) {
        if (parsed.is_param[i]) {
            assert(param_idx < parsed.param_names.size());
            params[parsed.param_names[param_idx++]] = std::string(path_segs[i]);
        } else if (parsed.segments[i] != path_segs[i]) {
            return false;
        }
    }

    assert(param_idx == parsed.param_names.size());
    return true;
}

const char* verb_to_string(boost::beast::http::verb method) {
    switch (method) {
        case boost::beast::http::verb::get:
            return "GET";
        case boost::beast::http::verb::post:
            return "POST";
        case boost::beast::http::verb::put:
            return "PUT";
        case boost::beast::http::verb::delete_:
            return "DELETE";
        case boost::beast::http::verb::patch:
            return "PATCH";
        default:
            return "UNKNOWN";
    }
}

}  // namespace

void router::add_http_route(boost::beast::http::verb method,
                            const std::string& pattern,
                            std::unique_ptr<endpoint::interface> endpoint) {
    route_entry entry;
    entry.method = method;
    entry.pattern = pattern;
    entry.parsed = parse_pattern(pattern);
    entry.endpoint = std::move(endpoint);

    if (m_logger) {
        m_logger->debug("Registered route {} {}", verb_to_string(method),
                        pattern);
    }

    m_http_routes.push_back(std::move(entry));
}

void router::add_ws_route(
    const std::string& pattern,
    std::unique_ptr<transport::ws_handler_interface> handler) {
    ws_route_entry entry;
    entry.pattern = pattern;
    entry.parsed = parse_pattern(pattern);
    entry.handler = std::move(handler);

    if (m_logger) {
        m_logger->debug("Registered WS route {}", pattern);
    }

    m_ws_routes.push_back(std::move(entry));
}

bool router::dispatch_http(boost::beast::http::verb method,
                           const std::string& target,
                           core::request_context& ctx,
                           const endpoint::http_request& req,
                           endpoint::response_sender sender) const {
    auto path_sv = strip_query(target);
    auto path_segs = tokenize_sv(path_sv);

    for (auto& entry : m_http_routes) {
        if (entry.method != method) continue;
        if (path_segs.size() != entry.parsed.segments.size()) continue;

        std::unordered_map<std::string, std::string> params;
        if (match_segments(path_segs, entry.parsed, params)) {
            if (m_logger) {
                m_logger->debug("Matched {} for {}", entry.pattern, path_sv);
            }
            ctx.path_params = std::move(params);
            entry.endpoint->invoke(std::move(ctx), req, std::move(sender));
            return true;
        }
    }

    if (m_logger) {
        m_logger->debug("No route for {}", path_sv);
    }
    return false;
}

transport::ws_handler_interface* router::match_ws(
    const std::string& target,
    std::unordered_map<std::string, std::string>& path_params) const {
    auto path_sv = strip_query(target);
    auto path_segs = tokenize_sv(path_sv);

    for (auto& entry : m_ws_routes) {
        if (path_segs.size() != entry.parsed.segments.size()) continue;

        std::unordered_map<std::string, std::string> params;
        if (match_segments(path_segs, entry.parsed, params)) {
            if (m_logger) {
                m_logger->debug("Matched WS {} for {}", entry.pattern, path_sv);
            }

            path_params = std::move(params);
            return entry.handler.get();
        }
    }

    if (m_logger) {
        m_logger->debug("No WS route for {}", path_sv);
    }

    return nullptr;
}

}  // namespace clover2_http::http::routing
