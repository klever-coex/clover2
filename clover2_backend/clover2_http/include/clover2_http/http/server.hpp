#pragma once

// clover2
#include "clover2_http/http/middleware/base_middleware.hpp"
#include <clover2_http/http/core/logger.hpp>
#include <clover2_http/http/endpoint/adapter.hpp>
#include <clover2_http/http/routing/router.hpp>
#include <clover2_http/http/transport/listener.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

// boost
#include <boost/asio/io_context.hpp>
#include <boost/beast/http/verb.hpp>

// STL
#include <format>
#include <memory>
#include <string>

namespace clover2_http::http {

/**
 * @brief HTTP/1.1 and WebSocket server.
 *
 * Owns the router and the TCP listener. Routes and middleware are registered
 * here and served after listen() is called. All I/O runs on the
 * caller-provided io_context; handlers and middleware are invoked on the
 * per-connection strand.
 */
class server {
public:
    /**
     * @brief Constructs the server with a default logger.
     *
     * @param io Asio context.
     */
    explicit server(boost::asio::io_context& io)
        : server(io, clover2_http::http::core::simple_logger("clover2_http")) {}

    /**
     * @brief Constructs the server with a custom logger.
     *
     * @param io Asio context.
     * @param log Logger.
     */
    template <typename L>
    explicit server(boost::asio::io_context& io, L&& log)
        : m_io(io)
        , m_logger(std::make_shared<std::decay_t<L>>(std::forward<L>(log)))
        , m_router(std::make_unique<routing::router>(m_logger)) {}

    /**
     * @brief Registers a GET route.
     *
     * @tparam Resp JSON compatable response type.
     * @tparam HandlerT void(core::request_context, endpoint::deferred_reply<Resp>).
     *
     * @param path Route pattern.
     * @param handler Handler invoked for the route.
     * @throws core::routing_error on duplicate route or invalid pattern.
     */
    template <typename Resp, typename HandlerT>
    void get(const std::string& path, HandlerT handler) {
        add_http_route<void, Resp>(boost::beast::http::verb::get, path,
                                   std::move(handler));
    }

    /**
     * @brief Registers a POST route.
     *
     * @tparam Req JSON compatable request type.
     * @tparam Resp JSON compatable response type.
     * @tparam HandlerT void(core::request_context, Req,
     *                       endpoint::deferred_reply<Resp>).
     *
     * @param path Route pattern.
     * @param handler Handler invoked for the route.
     * @throws core::routing_error on duplicate route or invalid pattern.
     */
    template <typename Req, typename Resp, typename HandlerT>
    void post(const std::string& path, HandlerT handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::post, path,
                                  std::move(handler));
    }

    /**
     * @brief Registers a PUT route.
     *
     * @tparam Req JSON compatible request type.
     * @tparam Resp JSON compatible response type.
     * @tparam HandlerT void(core::request_context, Req,
     *                       endpoint::deferred_reply<Resp>).
     *
     * @param path Route pattern.
     * @param handler Handler invoked for the route.
     * @throws core::routing_error on duplicate route or invalid pattern.
     */
    template <typename Req, typename Resp, typename HandlerT>
    void put(const std::string& path, HandlerT handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::put, path,
                                  std::move(handler));
    }

    /**
     * @brief Registers a DELETE route.
     *
     * @tparam Req JSON compatible request type.
     * @tparam Resp JSON compatible response type.
     * @tparam HandlerT void(core::request_context, Req,
     *                       endpoint::deferred_reply<Resp>).
     *
     * @param path Route pattern.
     * @param handler Handler invoked for the route.
     * @throws core::routing_error on duplicate route or invalid pattern.
     */
    template <typename Req, typename Resp, typename HandlerT>
    void del(const std::string& path, HandlerT handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::delete_, path,
                                  std::move(handler));
    }

    /**
     * @brief Registers a PATCH route.
     *
     * @tparam Req JSON compatible request type.
     * @tparam Resp JSON compatible response type.
     * @tparam HandlerT void(core::request_context, Req,
     *                       endpoint::deferred_reply<Resp>).
     *
     * @param path Route pattern.
     * @param handler Handler invoked for the route.
     * @throws core::routing_error on duplicate route or invalid pattern.
     */
    template <typename Req, typename Resp, typename HandlerT>
    void patch(const std::string& path, HandlerT handler) {
        add_http_route<Req, Resp>(boost::beast::http::verb::patch, path,
                                  std::move(handler));
    }

    /**
     * @brief Registers a typed JSON WebSocket endpoint.
     *
     * @tparam T JSON message type exchanged over the socket.
     * @tparam HandlerT void(std::shared_ptr<transport::ws_session<T>>).
     *
     * @param path Route pattern.
     * @param handler Handler invoked once per accepted connection.
     * @throws core::routing_error on duplicate route or invalid pattern.
     */
    template <typename T, typename HandlerT>
    void ws(const std::string& path, HandlerT handler) {
        auto ws_handler = std::make_unique<transport::ws_handler<T>>(
            std::move(handler), m_io, m_logger);
        m_router->add_ws_route(path, std::move(ws_handler));
    }

    /**
     * @brief Registers a raw WebSocket endpoint.
     *
     * @tparam HandlerT void(std::shared_ptr<transport::base_ws_session>).
     *
     * @param path Route pattern.
     * @param handler Handler invoked once per accepted connection.
     * @throws core::routing_error on duplicate route or invalid pattern.
     */
    template <typename HandlerT>
    void raw_ws(const std::string& path, HandlerT handler) {
        auto ws_handler = std::make_unique<transport::raw_ws_handler>(
            std::move(handler), m_io, m_logger);
        m_router->add_ws_route(path, std::move(ws_handler));
    }

    /**
     * @brief Registers a default-constructed middleware on a path prefix.
     *
     * @tparam MiddlewareT Type derived from middleware::base_middleware.
     *
     * @param pattern Prefix pattern; "/" applies to every request.
     * @throws core::routing_error on invalid pattern.
     */
    template <middleware::middleware MiddlewareT>
    void use(const std::string& pattern) {
        use(pattern, [] { return std::make_unique<MiddlewareT>(); });
    }

    /**
     * @brief Registers middleware on a path prefix via a factory.
     *
     * The factory runs once per request, so middleware may hold per-request
     * state. Multiple middleware on one pattern run in registration order;
     * middleware on different patterns run from the most general ("/") to
     * the most specific one. Middleware sees every HTTP request whose path
     * starts with the pattern, including ones without a matching route.
     *
     * @param pattern Prefix pattern; "/" applies to every request.
     * @param creator Factory returning a base_middleware instance.
     * @throws core::routing_error on invalid pattern.
     */
    void use(const std::string& pattern,
             routing::router::middleware_creator creator) {
        m_router->add_middleware(pattern, std::move(creator));
    }

    /**
     * @brief Start accepting connections.
     *
     * @param address Bind address.
     * @param port Bind port.
     * @throws std::runtime_error if the acceptor cannot bind.
     */
    void listen(const std::string& address, uint16_t port) {
        auto const addr = boost::asio::ip::tcp::endpoint(
            boost::asio::ip::make_address(address), port);
        m_listener = std::make_shared<transport::listener>(m_io, addr,
                                                           *m_router, m_logger);

        m_listener->start();
        m_logger->info("Listening on {}:{}", address, port);
    }

    /**
     * @brief Stops accepting new connections.
     */
    void stop() {
        if (m_listener) {
            m_listener->stop();
        }
    }

private:
    /**
     * @brief Wraps the handler in an endpoint adapter and registers the route.
     *
     * @tparam Req JSON compatible request type (void for none).
     * @tparam Resp JSON compatible response type.
     * @tparam Handler Handler type (see the route registration methods).
     *
     * @param method HTTP verb.
     * @param path Route pattern.
     * @param handler Handler invoked for the route.
     * @throws core::routing_error on duplicate route or invalid pattern.
     */
    template <typename Req, typename Resp, typename Handler>
    void add_http_route(boost::beast::http::verb method,
                        const std::string& path, Handler handler) {
        auto ep =
            std::make_unique<endpoint::adapter<Req, Resp>>(std::move(handler));
        m_router->add_http_route(method, path, std::move(ep));
    }

    boost::asio::io_context& m_io;  ///< Asio context of the server.
    std::shared_ptr<clover2_http::http::core::logger>
        m_logger;  ///< Logger shared with the router.
    std::unique_ptr<routing::router>
        m_router;  ///< Routes, WS endpoints and middleware.
    std::shared_ptr<transport::listener>
        m_listener;  ///< TCP acceptor (created by listen()).
};

}  // namespace clover2_http::http
