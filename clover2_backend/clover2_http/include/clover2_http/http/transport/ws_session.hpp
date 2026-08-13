#pragma once

#include <clover2_http/http/core/request_context.hpp>
#include <clover2_http/http/transport/base_ws_session.hpp>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

namespace clover2_http::http::transport {

template <typename T>
class ws_session : public std::enable_shared_from_this<ws_session<T>> {
public:
    using message_handler =
        std::function<void(std::shared_ptr<ws_session<T>>, T)>;
    using close_handler =
        std::function<void(std::shared_ptr<ws_session<T>>, int)>;
    using connection_handler =
        std::function<void(std::shared_ptr<ws_session<T>>)>;

    explicit ws_session(boost::asio::ip::tcp::socket socket,
                        boost::asio::io_context& io)
        : m_raw(std::make_shared<base_ws_session>(std::move(socket), io)) {}

    void start(
        boost::beast::http::request<boost::beast::http::string_body> request,
        core::request_context ctx, connection_handler handler) {
        m_raw->start(std::move(request), std::move(ctx),
                     [self = this->shared_from_this(),
                      handler = std::move(handler)](auto) {
                         if (handler) handler(self);
                     });
    }

    void on_message(message_handler handler) {
        m_msg_handler = std::move(handler);
        m_raw->on_text([self = this->shared_from_this()](
                           std::shared_ptr<base_ws_session>, std::string data) {
            T msg = nlohmann::json::parse(data).get<T>();
            if (self->m_msg_handler) {
                self->m_msg_handler(self, std::move(msg));
            }
        });
    }

    void on_close(close_handler handler) {
        m_close_handler = std::move(handler);
        m_raw->on_close([self = this->shared_from_this()](
                            std::shared_ptr<base_ws_session>, int code) {
            if (self->m_close_handler) {
                self->m_close_handler(self, code);
            }
        });
    }

    void start_reading() { m_raw->start_reading(); }

    void write(T message) {
        auto jv = nlohmann::json(std::move(message));
        m_raw->write_text(jv.dump());
    }

    void close(int code = 1000, const std::string& reason = "") {
        m_raw->close(code, reason);
    }

    bool is_open() const { return m_raw->is_open(); }

    core::request_context& context() { return m_raw->context(); }

    std::shared_ptr<base_ws_session> raw() { return m_raw; }

private:
    std::shared_ptr<base_ws_session> m_raw;
    message_handler m_msg_handler;
    close_handler m_close_handler;
};

}  // namespace clover2_http::http::transport
