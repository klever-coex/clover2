#pragma once

#include <clover2_http/http/core/logger.hpp>
#include <clover2_http/http/endpoint/adapter.hpp>
#include <clover2_http/http/routing/router.hpp>
#include <clover2_http/http/transport/listener.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/beast/http/verb.hpp>

#include <format>
#include <functional>
#include <memory>
#include <string>

namespace clover2_http::http {

class server {
public:
    explicit server(boost::asio::io_context& io)
        : server(io, clover2_http::http::core::simple_logger("clover2_http")) {}

    template <typename L>
    explicit server(boost::asio::io_context& io, L&& log)
        : m_io(io)
        , m_logger(std::make_shared<std::decay_t<L>>(std::forward<L>(log)))
        , m_router(std::make_unique<routing::router>(m_logger)) {}

    template <typename Req, typename Resp, typename Handler>
    void get(const std::string& path, Handler handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::get, path,
                                  std::move(handler));
    }

    template <typename Req, typename Resp, typename Handler>
    void post(const std::string& path, Handler handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::post, path,
                                  std::move(handler));
    }

    template <typename Req, typename Resp, typename Handler>
    void put(const std::string& path, Handler handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::put, path,
                                  std::move(handler));
    }

    template <typename Req, typename Resp, typename Handler>
    void del(const std::string& path, Handler handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::delete_, path,
                                  std::move(handler));
    }

    template <typename Req, typename Resp, typename Handler>
    void patch(const std::string& path, Handler handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::patch, path,
                                  std::move(handler));
    }

    template <typename T, typename Handler>
    void ws(const std::string& path, Handler handler) {
        auto ws_handler = std::make_unique<transport::ws_handler<T>>(
            std::move(handler), m_io);
        m_router->add_ws_route(path, std::move(ws_handler));
    }

    void listen(const std::string& address, uint16_t port) {
        auto const addr = boost::asio::ip::tcp::endpoint(
            boost::asio::ip::make_address(address), port);
        m_listener = std::make_shared<transport::listener>(m_io, addr,
                                                           *m_router, m_logger);
        m_listener->start();
        m_logger->info("Listening on {}:{}", address, port);
    }

private:
    template <typename Req, typename Resp, typename Handler>
    void add_http_route(boost::beast::http::verb method,
                        const std::string& path, Handler handler) {
        auto ep =
            std::make_unique<endpoint::adapter<Req, Resp>>(std::move(handler));
        m_router->add_http_route(method, path, std::move(ep));
    }

    boost::asio::io_context& m_io;
    std::shared_ptr<clover2_http::http::core::logger> m_logger;
    std::unique_ptr<routing::router> m_router;
    std::shared_ptr<transport::listener> m_listener;
};

}  // namespace clover2_http::http
