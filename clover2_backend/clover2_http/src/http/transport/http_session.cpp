#include <clover2_http/http/routing/router.hpp>
#include <clover2_http/http/transport/http_session.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/websocket.hpp>

#include <algorithm>
#include <cctype>

namespace clover2_http::http::transport {

namespace {

constexpr std::size_t k_max_body_size = 10 * 1024 * 1024;  // 10 MiB
constexpr std::chrono::seconds k_idle_timeout{30};

}  // namespace

http_session::http_session(boost::asio::ip::tcp::socket socket,
                           routing::router& router, boost::asio::io_context& io,
                           std::shared_ptr<core::logger> log)
    : m_socket(std::move(socket))
    , m_strand(boost::asio::make_strand(io))
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

    if (!m_upgraded) {
        m_logger->debug("Session close");
    }
}

void http_session::start() { do_read(); }

void http_session::do_read() {
    m_request = {};
    m_buffer.clear();

    m_timer.expires_after(k_idle_timeout);
    m_timer.async_wait(boost::asio::bind_executor(
        m_strand, [self = shared_from_this()](boost::system::error_code ec) {
            if (!ec) {
                self->m_logger->debug("Idle timeout, closing session");
                self->do_close();
            }
        }));

    auto parser = std::make_shared<request_parser_t>();
    parser->body_limit(k_max_body_size);

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
        m_logger->warn("Request body exceeds the {} bytes limit",
                       k_max_body_size);
        send_error(413, "Payload Too Large");
        return;
    }

    if (ec) {
        m_logger->warn("Read error: {}", ec.message());
        do_close();
        return;
    }

    m_request = parser->release();
    m_keep_alive = m_request.keep_alive();
    m_version = m_request.version();

    handle_request();
}

void http_session::handle_request() {
    try {
        const std::string target_str = m_request.target();
        auto parsed = boost::urls::parse_relative_ref(target_str);

        if (parsed.has_error()) {
            parsed = boost::urls::parse_absolute_uri(target_str);
        }

        if (parsed.has_error()) {
            m_logger->warn("Malformed request target: {}", target_str);
            send_error(400, "Malformed request target");
            return;
        }

        boost::urls::url_view uv = *parsed;

        if (boost::beast::websocket::is_upgrade(m_request)) {
            std::unordered_map<std::string, std::string> path_params;
            auto* ws_handler = m_router.match_ws(uv, path_params);

            if (ws_handler) {
                auto ctx = make_context(uv);
                ctx.path_params = std::move(path_params);
                m_upgraded = true;
                m_logger->debug("Session upgraded to WebSocket");
                ws_handler->on_accept(std::move(m_socket),   //
                                      std::move(m_request),  //
                                      std::move(ctx));

                m_logger->info("Open WebSocket: {} from {}", target_str,
                               ctx.remote_endpoint.address().to_string());
                return;
            }

            send_error(404, "WebSocket endpoint not found");
            return;
        }

        auto ctx = make_context(uv);

        m_logger->info("Handling request: {} {} from {}",
                       std::string(m_request.method_string()), target_str,
                       ctx.remote_endpoint.address().to_string());

        auto method = m_request.method();
        m_router.dispatch_http(
            method, uv, ctx, std::move(m_request),
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
        send_error(500, "Internal Server Error");
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

core::request_context http_session::make_context(boost::urls::url_view url) {
    core::request_context ctx(url);

    boost::system::error_code ec;
    ctx.remote_endpoint = m_socket.remote_endpoint(ec);
    if (ec) {
        m_logger->warn("remote_endpoint unavailable: {}", ec.message());
    }

    for (const auto& f : m_request) {
        std::string name(f.name_string());
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        ctx.headers[std::move(name)] = std::string(f.value());
    }

    return ctx;
}

}  // namespace clover2_http::http::transport
