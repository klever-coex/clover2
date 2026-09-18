// clover2
#include <clover2_http/http/routing/router.hpp>
#include <clover2_http/http/transport/http_session.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

// boost
#include <boost/asio/bind_executor.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/websocket.hpp>

// STL
#include <algorithm>
#include <cctype>
#include <optional>
#include <string>

namespace clover2_http::http::transport {

http_session::http_session(boost::asio::ip::tcp::socket socket,
                           routing::router& router, boost::asio::io_context& io,
                           std::shared_ptr<core::logger> log,
                           const core::settings& settings)
    : m_settings(settings)
    , m_strand(boost::asio::make_strand(io))
    , m_socket(std::move(socket))
    , m_router(router)
    , m_timer(m_strand)
    , m_logger(std::move(log)) {
    boost::system::error_code ec;
    auto ep = m_socket.remote_endpoint(ec);

    if (!ec) {
        m_logger->debug("Session open from {}: {}", ep.address().to_string(),
                        ep.port());
    } else {
        m_logger->warn("Session open from unknown address: {}", ec.message());
    }
}

http_session::~http_session() {
    boost::system::error_code ec;
    m_timer.cancel(ec);

    m_logger->info("Session close");

    if (!m_upgraded) {
        m_logger->debug("Session close");
    }
}

void http_session::start() { do_read(); }

void http_session::do_read() {
    m_buffer.clear();

    m_timer.expires_after(m_settings.http_timeout());
    m_timer.async_wait(boost::asio::bind_executor(
        m_strand, [self = shared_from_this()](boost::system::error_code ec) {
            if (!ec) {
                self->m_logger->warn("Idle timeout, closing http session");
                self->do_close();
            }
        }));

    auto parser = std::make_shared<request_parser_t>();
    parser->body_limit(m_settings.max_body_size());

    boost::beast::http::async_read(
        m_socket, m_buffer, *parser,
        boost::asio::bind_executor(
            m_strand, [self = shared_from_this(), parser](
                          boost::system::error_code ec, std::size_t n) {
                self->m_timer.cancel();
                self->on_read(ec, n, parser);
            }));
}

void http_session::on_read(boost::beast::error_code ec, std::size_t,
                           std::shared_ptr<request_parser_t> parser) {
    if (ec == boost::beast::http::error::end_of_stream) {
        do_close();
        return;
    }

    if (ec == boost::asio::error::operation_aborted) {
        do_close();
        return;
    }

    if (ec == boost::beast::http::error::body_limit) {
        m_keep_alive = false;
        m_logger->warn("Request body exceeds the {} bytes limit",
                       m_settings.max_body_size());
        send_error(413, "Payload Too Large");
        return;
    }

    if (ec) {
        m_logger->warn("Read error: {}", ec.message());
        do_close();
        return;
    }

    auto request = parser->release();
    m_keep_alive = request.keep_alive();
    m_version = request.version();

    auto uv = parse_url(request);

    if (uv.has_value()) {
        handle_request(std::move(request), uv.value());
    }
}

void http_session::handle_request(request_t request, boost::urls::url_view uv) {
    try {
        if (boost::beast::websocket::is_upgrade(request)) {
            handle_websocket(std::move(request), uv);
            return;
        }

        auto ctx = make_context(request, uv);

        m_logger->info("Handling request: {} {} from {}",
                       std::string(request.method_string()),
                       std::string(uv.encoded_target()),
                       ctx.remote_endpoint.address().to_string());

        auto method = request.method();
        m_router.dispatch_http(
            method, uv, ctx, std::move(request),
            [self = shared_from_this()](
                boost::beast::http::response<boost::beast::http::string_body>
                    response) {
                boost::asio::post(
                    self->m_strand,
                    [self, response = std::move(response)]() mutable {
                        response.keep_alive(self->m_keep_alive);
                        response.version(self->m_version);
                        self->do_write(std::move(response));
                    });
            });

    } catch (const core::http_error& e) {
        send_error(e.status(), e.message());
    } catch (const std::exception& e) {
        m_logger->error("Exception in request handling: {}", e.what());
        send_error(500, "Fatal Server Error");
    }
}

void http_session::handle_websocket(request_t request,
                                    boost::urls::url_view uv) {
    std::unordered_map<std::string, std::string> path_params;
    auto* ws_handler = m_router.match_ws(uv, path_params);

    if (ws_handler) {
        auto ctx = make_context(request, uv);
        ctx.path_params = std::move(path_params);
        m_upgraded = true;
        m_logger->info("Open WebSocket: {} from {}",
                       std::string(uv.encoded_target()),
                       ctx.remote_endpoint.address().to_string());

        ws_handler->on_accept(std::move(m_socket),  //
                              std::move(request),   //
                              std::move(ctx));
    } else {
        send_error(404, "WebSocket endpoint not found");
    }
}

void http_session::send_error(int status, const std::string& message) {
    endpoint::http_response resp;
    endpoint::make_error_response(resp, status, message);

    resp.keep_alive(m_keep_alive);
    resp.version(m_version);

    do_write(std::move(resp));
}

void http_session::do_write(
    boost::beast::http::response<boost::beast::http::string_body> response) {
    m_response = std::move(response);
    bool close = !m_keep_alive || m_response.need_eof();

    auto self = shared_from_this();
    boost::beast::http::async_write(
        m_socket, m_response,
        boost::asio::bind_executor(
            m_strand,
            [self, close](boost::system::error_code ec, std::size_t n) {
                self->on_write(ec, n, close);
            }));
}

void http_session::on_write(boost::beast::error_code ec, std::size_t,
                            bool close) {
    if (ec) {
        do_close();
        return;
    }

    if (close) {
        do_close();
        return;
    }

    do_read();
}

void http_session::do_close() {
    boost::system::error_code ec;
    m_timer.cancel(ec);

    ec = m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    ec = m_socket.close(ec);
}

std::optional<boost::urls::url> http_session::parse_url(
    const request_t& request) {
    const std::string target_str = request.target();
    auto parsed = boost::urls::parse_uri_reference(target_str);

    if (parsed.has_error()) {
        m_logger->warn("Malformed request target: {}", target_str);
        send_error(400, "Malformed request target");
        return std::nullopt;
    }

    return boost::urls::url(*parsed);
}

core::request_context http_session::make_context(const request_t& request,
                                                 boost::urls::url_view url) {
    core::request_context ctx(url);

    boost::system::error_code ec;
    ctx.remote_endpoint = m_socket.remote_endpoint(ec);
    if (ec) {
        m_logger->warn("remote_endpoint unavailable: {}", ec.message());
    }

    for (const auto& f : request) {
        std::string name(f.name_string());
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        ctx.headers[std::move(name)] = std::string(f.value());
    }

    return ctx;
}

}  // namespace clover2_http::http::transport
