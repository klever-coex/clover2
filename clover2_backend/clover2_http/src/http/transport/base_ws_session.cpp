#include <clover2_http/http/transport/base_ws_session.hpp>

#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/websocket/error.hpp>

namespace clover2_http::http::transport {

base_ws_session::base_ws_session(boost::asio::ip::tcp::socket socket,
                                 boost::asio::io_context& io)
    : m_ws(std::move(socket))
    , m_strand(boost::asio::make_strand(io)) {}

base_ws_session::~base_ws_session() = default;

void base_ws_session::start(
    boost::beast::http::request<boost::beast::http::string_body> request,
    core::request_context ctx, connection_handler handler) {
    m_ctx = std::move(ctx);

    m_ws.async_accept(
        request,
        boost::asio::bind_executor(
            m_strand, [self = shared_from_this(), handler = std::move(handler)](
                          boost::system::error_code ec) mutable {
                if (ec) return;

                if (handler) {
                    handler(self);
                }
            }));
}

void base_ws_session::on_text(text_handler handler) {
    m_text_handler = std::move(handler);
}

void base_ws_session::on_binary(binary_handler handler) {
    m_binary_handler = std::move(handler);
}

void base_ws_session::on_close(close_handler handler) {
    m_close_handler = std::move(handler);
}

void base_ws_session::start_reading() { do_read(); }

void base_ws_session::write_text(std::string data) {
    boost::asio::post(m_strand, [self = shared_from_this(),
                                 data = std::move(data)]() mutable {
        if (!self->m_ws.is_open()) return;

        self->m_write_queue.push_back(queued_message{std::move(data), false});

        if (!self->m_writing) {
            self->m_writing = true;
            self->do_write();
        }
    });
}

void base_ws_session::write_binary(std::vector<uint8_t> data) {
    write_raw(
        std::string(reinterpret_cast<const char*>(data.data()), data.size()),
        true);
}

void base_ws_session::write_binary(const uint8_t* data, size_t size) {
    write_raw(std::string(reinterpret_cast<const char*>(data), size), true);
}

void base_ws_session::close(int code, const std::string& reason) {
    boost::asio::dispatch(m_strand,
                          [self = shared_from_this(), code, reason]() {
                              if (self->m_closed) return;

                              self->m_closed = true;
                              boost::system::error_code ec;

                              self->m_ws.close(
                                  boost::beast::websocket::close_reason{
                                      boost::beast::websocket::close_code{
                                          static_cast<uint16_t>(code)},
                                      reason},
                                  ec);
                          });
}

void base_ws_session::ping(std::string payload) {
    boost::asio::post(m_strand, [self = shared_from_this(),
                                 p = std::move(payload)]() mutable {
        if (!self->m_ws.is_open()) return;

        self->m_ws.async_ping(
            boost::beast::websocket::ping_data{p},
            boost::asio::bind_executor(self->m_strand,
                                       [self](boost::system::error_code) {
                                           // fire-and-forget
                                       }));
    });
}

bool base_ws_session::is_open() const { return m_ws.is_open(); }

core::request_context& base_ws_session::context() { return m_ctx; }

void base_ws_session::write_raw(std::string data, bool binary) {
    boost::asio::post(m_strand, [self = shared_from_this(),
                                 data = std::move(data), binary]() mutable {
        if (!self->m_ws.is_open()) return;

        self->m_write_queue.push_back(queued_message{std::move(data), binary});

        if (!self->m_writing) {
            self->m_writing = true;
            self->do_write();
        }
    });
}

void base_ws_session::do_read() {
    m_buffer.clear();
    m_ws.async_read(
        m_buffer, boost::asio::bind_executor(
                      m_strand, [self = shared_from_this()](
                                    boost::system::error_code ec, std::size_t) {
                          self->on_read(ec);
                      }));
}

void base_ws_session::on_read(boost::system::error_code ec) {
    if (ec == boost::beast::websocket::error::closed) {
        if (!m_closed && m_close_handler) {
            m_closed = true;
            m_close_handler(shared_from_this(), 1000);
        }

        return;
    }

    if (ec) {
        if (!m_closed && m_close_handler) {
            m_closed = true;
            m_close_handler(shared_from_this(), 1006);
        }

        return;
    }

    auto data = boost::beast::buffers_to_string(m_buffer.data());
    bool is_binary = m_ws.got_binary();

    if (is_binary) {
        dispatch_binary(std::move(data));
    } else {
        dispatch_text(std::move(data));
    }
}

void base_ws_session::dispatch_binary(std::string data) {
    try {
        if (m_binary_handler) {
            std::vector<uint8_t> bytes(data.begin(), data.end());
            m_binary_handler(shared_from_this(), std::move(bytes));
        } else {
            if (!m_closed) {
                auto ws_code =
                    boost::beast::websocket::close_code::unknown_data;
                m_closed = true;

                if (m_close_handler) {
                    m_close_handler(shared_from_this(), ws_code);
                }

                do_close_ws(ws_code);
            }
            return;
        }
    } catch (const std::exception&) {
        if (!m_closed) {
            auto ws_code = boost::beast::websocket::close_code::unknown_data;
            m_closed = true;

            if (m_close_handler) {
                m_close_handler(shared_from_this(), ws_code);
            }

            do_close_ws(ws_code);
        }

        return;
    }

    do_read();
}

void base_ws_session::dispatch_text(std::string data) {
    try {
        if (m_text_handler) {
            m_text_handler(shared_from_this(), std::move(data));
        }

    } catch (const std::exception&) {
        if (!m_closed) {
            m_closed = true;

            if (m_close_handler) {
                m_close_handler(shared_from_this(), 1003);
            }

            do_close_ws(boost::beast::websocket::close_code::unknown_data);
        }

        return;
    }

    do_read();
}

void base_ws_session::do_write() {
    auto& msg = m_write_queue.front();
    m_ws.text(!msg.is_binary);

    m_ws.async_write(
        boost::asio::buffer(msg.data),
        boost::asio::bind_executor(
            m_strand, [self = shared_from_this()](boost::system::error_code ec,
                                                  std::size_t) {
                if (ec) {
                    self->m_writing = false;
                    self->m_write_queue.clear();

                    if (!self->m_closed) {
                        self->m_closed = true;

                        if (self->m_close_handler) {
                            self->m_close_handler(self, 1006);
                        }

                        self->do_close_ws();
                    }

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

void base_ws_session::do_close_ws(boost::beast::websocket::close_code code) {
    boost::system::error_code ec;
    m_ws.close(code, ec);
}

}  // namespace clover2_http::http::transport
