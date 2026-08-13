#include <clover2_http/http/routing/router.hpp>
#include <clover2_http/http/transport/http_session.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/websocket.hpp>

#include <iostream>
#include <sstream>

namespace clover2_http::http::transport {

parsed_target parse_target(const std::string& target) {
    parsed_target pt;
    auto qpos = target.find('?');
    if (qpos != std::string::npos) {
        pt.path = target.substr(0, qpos);
        auto query = target.substr(qpos + 1);
        std::istringstream stream(query);
        std::string pair;
        while (std::getline(stream, pair, '&')) {
            auto epos = pair.find('=');
            if (epos != std::string::npos) {
                pt.query_params[pair.substr(0, epos)] = pair.substr(epos + 1);
            } else {
                pt.query_params[pair] = "";
            }
        }
    } else {
        pt.path = target;
    }
    return pt;
}

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
        m_logger->info("Session open from {}: {}", ep.address().to_string(),
                       ep.port());
    }
}

http_session::~http_session() {
    boost::system::error_code ec;
    m_timer.cancel(ec);
    m_logger->debug("Session close");
}

void http_session::start() { do_read(); }

void http_session::do_read() {
    m_request = {};
    m_buffer.clear();
    m_close = false;

    m_timer.expires_after(std::chrono::seconds(30));
    m_timer.async_wait(boost::asio::bind_executor(
        m_strand, [self = shared_from_this()](boost::system::error_code ec) {
            if (!ec) {
                self->do_close();
            }
        }));

    boost::beast::http::async_read(
        m_socket, m_buffer, m_request,
        boost::asio::bind_executor(
            m_strand, [self = shared_from_this()](boost::system::error_code ec,
                                                  std::size_t n) {
                self->m_timer.cancel();
                self->on_read(ec, n);
            }));
}

void http_session::on_read(boost::beast::error_code ec, std::size_t) {
    if (ec == boost::beast::http::error::end_of_stream) {
        do_close();
        return;
    }

    if (ec) {
        m_logger->warn("Read error: {}", ec.message());
        do_close();
        return;
    }

    m_keep_alive = m_request.keep_alive();
    m_version = m_request.version();

    handle_request();
}

void http_session::handle_request() {
    try {
        const std::string target_str = m_request.target();
        auto pt = parse_target(target_str);

        if (boost::beast::websocket::is_upgrade(m_request)) {

            std::unordered_map<std::string, std::string> path_params;
            auto* ws_handler = m_router.match_ws(pt.path, path_params);

            if (ws_handler) {
                auto ctx = make_context(pt);
                ctx.path_params = std::move(path_params);
                ws_handler->on_accept(std::move(m_socket), std::move(m_request),
                                      std::move(ctx));
                return;
            }

            send_error(404, "WebSocket endpoint not found");
            return;
        }

        auto ctx = make_context(pt);

        m_logger->debug("Handling request: {} {} from {}",
                         std::string(m_request.method_string()), target_str,
                         ctx.remote_endpoint.address().to_string());

        auto found = m_router.dispatch_http(
            m_request.method(), target_str, ctx, m_request,
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

        if (!found) {
            send_error(404, "Not Found");
        }
    } catch (const core::http_error& e) {
        send_error(e.status(), e.message());
    } catch (const std::exception& e) {
        m_logger->error("Exception in request handling: {}", e.what());
        send_error(500, "Internal Server Error");
    }
}

void http_session::send_error(int status, const std::string& message) {
    auto resp = endpoint::make_error_response(status, message);
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
    m_socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    m_socket.close(ec);
}

core::request_context http_session::make_context(const parsed_target& pt) {
    core::request_context ctx;
    boost::system::error_code ec;
    ctx.remote_endpoint = m_socket.remote_endpoint(ec);

    for (auto const& f : m_request) {
        ctx.headers[std::string(f.name_string())] = std::string(f.value());
    }

    ctx.query_params = pt.query_params;
    return ctx;
}

core::request_context http_session::make_context(boost::urls::url_view url) {
    core::request_context ctx;
    boost::system::error_code ec;
    ctx.remote_endpoint = m_socket.remote_endpoint(ec);

    for (const auto& f : m_request) {
        ctx.headers[std::string(f.name_string())] = std::string(f.value());
    }

    for (auto q : url.params()) {
        ctx.query_params[q.key] = q.value;
    }

    return ctx;
}

}  // namespace clover2_http::http::transport
