#pragma once

#include <clover2_http/core/request_context.hpp>
#include <clover2_http/serialization/json_traits.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/websocket/error.hpp>
#include <boost/beast/websocket/stream.hpp>
#include <nlohmann/json.hpp>

#include <deque>
#include <functional>
#include <memory>
#include <string>

namespace clover2_http::transport {

template <typename T>
class ws_session : public std::enable_shared_from_this<ws_session<T>> {
public:
    using message_handler =
        std::function<void(std::shared_ptr<ws_session<T>>, T)>;
    using close_handler =
        std::function<void(std::shared_ptr<ws_session<T>>, int)>;
    using connection_handler =
        std::function<void(std::shared_ptr<ws_session<T>>)>;

    using strand_type =
        boost::asio::strand<boost::asio::io_context::executor_type>;

    explicit ws_session(boost::asio::ip::tcp::socket socket,
                        boost::asio::io_context& io)
        : m_ws(std::move(socket))
        , m_strand(boost::asio::make_strand(io)) {}

    void start(
        boost::beast::http::request<boost::beast::http::string_body> request,
        core::request_context ctx, connection_handler handler) {
        m_ctx = std::move(ctx);
        m_ws.async_accept(
            request, boost::asio::bind_executor(
                         m_strand, [self = this->shared_from_this(),
                                    handler = std::move(handler)](
                                       boost::system::error_code ec) mutable {
                             if (ec) return;
                             if (handler) {
                                 handler(self);
                             }
                         }));
    }

    void on_message(message_handler handler) {
        m_msg_handler = std::move(handler);
    }

    void on_close(close_handler handler) {
        m_close_handler = std::move(handler);
    }

    void start_reading() { do_read(); }

    void write(T message) {
        boost::asio::post(m_strand, [self = this->shared_from_this(),
                                     msg = std::move(message)]() mutable {
            nlohmann::json jv;
            serialization::json_traits<T>::to_json(jv, msg);
            self->m_write_queue.push_back(jv.dump());
            if (!self->m_writing) {
                self->m_writing = true;
                self->do_write();
            }
        });
    }

    void close(int code = 1000, const std::string& reason = "") {
        boost::asio::post(
            m_strand, [self = this->shared_from_this(), code, reason]() {
                boost::system::error_code ec;
                self->m_ws.close(boost::beast::websocket::close_code{code}, ec);
            });
    }

    core::request_context& context() { return m_ctx; }

private:
    void do_read() {
        m_buffer.clear();
        auto self = this->shared_from_this();
        m_ws.async_read(
            m_buffer,
            boost::asio::bind_executor(
                m_strand, [self](boost::system::error_code ec, std::size_t) {
                    self->on_read(ec);
                }));
    }

    void on_read(boost::system::error_code ec) {
        if (ec == boost::beast::websocket::error::closed) {
            if (m_close_handler) {
                m_close_handler(this->shared_from_this(), 1000);
            }
            return;
        }
        if (ec) {
            if (m_close_handler) {
                m_close_handler(this->shared_from_this(), 1006);
            }
            return;
        }

        try {
            auto data = boost::beast::buffers_to_string(m_buffer.data());
            auto jv = nlohmann::json::parse(data);
            auto msg = serialization::impl::deserialize<T>(jv);

            if (m_msg_handler) {
                m_msg_handler(this->shared_from_this(), std::move(msg));
            }
        } catch (const std::exception&) {
            if (m_close_handler) {
                m_close_handler(this->shared_from_this(), 1003);
            }
            do_close_ws();
            return;
        }

        do_read();
    }

    void do_write() {
        auto& msg = m_write_queue.front();
        m_ws.text(true);

        auto self = this->shared_from_this();
        m_ws.async_write(
            boost::asio::buffer(msg),
            boost::asio::bind_executor(
                m_strand, [self](boost::system::error_code ec, std::size_t) {
                    if (ec) {
                        self->do_close_ws();
                        return;
                    }
                    self->m_write_queue.pop_front();
                    if (!self->m_write_queue.empty()) {
                        self->do_write();
                    } else {
                        self->m_writing = false;
                    }
                }));
    }

    void do_close_ws() {
        boost::system::error_code ec;
        m_ws.close(boost::beast::websocket::close_code::normal, ec);
    }

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;
    strand_type m_strand;
    boost::beast::flat_buffer m_buffer;
    core::request_context m_ctx;

    message_handler m_msg_handler;
    close_handler m_close_handler;

    std::deque<std::string> m_write_queue;
    bool m_writing = false;
};

}  // namespace clover2_http::transport
