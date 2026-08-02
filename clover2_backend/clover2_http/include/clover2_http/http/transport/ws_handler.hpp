#pragma once

#include <clover2_http/http/core/request_context.hpp>
#include <clover2_http/http/transport/ws_session.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

#include <functional>
#include <memory>

namespace clover2_http::http::transport {

class ws_handler_interface {
public:
    virtual ~ws_handler_interface() = default;
    virtual void on_accept(
        boost::asio::ip::tcp::socket socket,
        boost::beast::http::request<boost::beast::http::string_body> request,
        core::request_context ctx) = 0;
};

template <typename T>
class ws_handler : public ws_handler_interface {
public:
    using handler = std::function<void(std::shared_ptr<ws_session<T>>)>;

    explicit ws_handler(handler h, boost::asio::io_context& io)
        : m_handler(std::move(h))
        , m_io(io) {}

    void on_accept(
        boost::asio::ip::tcp::socket socket,
        boost::beast::http::request<boost::beast::http::string_body> request,
        core::request_context ctx) override {
        auto session = std::make_shared<ws_session<T>>(std::move(socket), m_io);

        session->start(std::move(request), std::move(ctx), m_handler);
    }

private:
    handler m_handler;
    boost::asio::io_context& m_io;
};

}  // namespace clover2_http::http::transport
