// clover2
#include <clover2_http/http/transport/base_ws_session.hpp>

// boost
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/post.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/websocket/error.hpp>

namespace clover2_http::http::transport {

base_ws_session::base_ws_session(boost::asio::ip::tcp::socket socket,
                                 boost::asio::io_context& io,
                                 std::shared_ptr<core::logger> log,
                                 const core::settings& settings)
    : m_settings(settings)
    , m_ws(std::move(socket))
    , m_strand(boost::asio::make_strand(io))
    , m_timer(m_strand)
    , m_logger(std::move(log)) {}

base_ws_session::~base_ws_session() {
    boost::system::error_code ec;
    m_timer.cancel(ec);
}

void base_ws_session::start(
    boost::beast::http::request<boost::beast::http::string_body> request,
    core::request_context ctx, connection_handler handler) {
    m_ctx = std::move(ctx);

    m_ws.async_accept(
        request,
        boost::asio::bind_executor(
            m_strand, [self = shared_from_this(), handler = std::move(handler)](
                          boost::system::error_code ec) mutable {
                if (ec) {
                    self->m_logger->warn("WebSocket accept error: {}",
                                         ec.message());
                    return;
                }

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
    write_raw(std::move(data), false);
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
    boost::asio::dispatch(m_strand, [self = shared_from_this(), code, reason] {
        self->prepare_close(code, boost::beast::websocket::close_reason{
                                      boost::beast::websocket::close_code{
                                          static_cast<uint16_t>(code)},
                                      reason});
    });
}

void base_ws_session::ping(std::string payload) {
    boost::asio::post(m_strand, [self = shared_from_this(),
                                 p = std::move(payload)]() mutable {
        if (self->m_closed || !self->is_open()) return;

        self->m_ws.async_ping(
            boost::beast::websocket::ping_data{p},
            boost::asio::bind_executor(self->m_strand,
                                       [self](boost::system::error_code) {}));
    });
}

bool base_ws_session::is_open() const { return m_ws.is_open(); }

core::request_context& base_ws_session::context() { return m_ctx; }

void base_ws_session::write_raw(std::string data, bool binary) {
    boost::asio::post(m_strand, [self = shared_from_this(),
                                 data = std::move(data), binary]() mutable {
        if (self->m_closed || !self->is_open()) return;

        self->reset_timer();
        self->m_write_queue.push_back(queued_message{std::move(data), binary});

        if (!self->m_writing) {
            self->m_writing = true;
            self->do_write();
        }
    });
}

void base_ws_session::reset_timer() {
    m_timer.expires_after(m_settings.websocket_timeout());
    m_timer.async_wait(boost::asio::bind_executor(
        m_strand, [self = shared_from_this()](boost::system::error_code ec) {
            if (ec) return;

            self->m_logger->debug("WS idle timeout, closing websocket session");
            self->fail(1001, boost::beast::websocket::close_code::going_away);
        }));
}

void base_ws_session::do_read() {
    m_buffer.clear();
    reset_timer();

    m_ws.async_read(
        m_buffer, boost::asio::bind_executor(
                      m_strand, [self = shared_from_this()](
                                    boost::system::error_code ec, std::size_t) {
                          self->on_read(ec);
                      }));
}

void base_ws_session::on_read(boost::system::error_code ec) {
    if (ec) {
        fail(ec == boost::beast::websocket::error::closed ? 1000 : 1006);
        return;
    }

    dispatch_data(m_buffer, m_ws.got_binary());
}

void base_ws_session::dispatch_data(const boost::beast::flat_buffer& buffer,
                                    bool is_binary) {
    try {
        if (m_binary_handler && is_binary) {
            std::vector<uint8_t> vec(buffer.size());
            boost::asio::buffer_copy(boost::asio::buffer(vec), buffer.data());

            m_binary_handler(shared_from_this(), std::move(vec));
        }

        if (m_text_handler && !is_binary) {
            auto data = boost::beast::buffers_to_string(buffer.data());

            m_text_handler(shared_from_this(), std::move(data));
        }
    } catch (const std::exception&) {
        fail(1003, boost::beast::websocket::close_code::unknown_data);
        return;
    }

    do_read();
}

void base_ws_session::do_write() {
    auto& msg = m_write_queue.front();
    m_ws.binary(msg.is_binary);

    m_ws.async_write(
        boost::asio::buffer(msg.data),
        boost::asio::bind_executor(
            m_strand, [self = shared_from_this()](boost::system::error_code ec,
                                                  std::size_t) {
                if (ec) {
                    self->m_write_queue.clear();
                    self->m_writing = false;
                    self->fail(1006);
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

void base_ws_session::prepare_close(
    int handler_code, boost::beast::websocket::close_reason reason) {
    if (m_closed) return;
    m_closed = true;

    m_write_queue.clear();
    m_writing = false;

    boost::system::error_code ec;
    m_timer.cancel(ec);
    ec = m_ws.next_layer().cancel(ec);

    m_ws.async_close(reason,
                     boost::asio::bind_executor(
                         m_strand, [self = shared_from_this(),
                                    handler_code](boost::system::error_code) {
                             if (self->m_close_handler) {
                                 self->m_close_handler(self, handler_code);
                             }
                         }));
}

void base_ws_session::fail(int code, boost::beast::websocket::close_code cc) {
    prepare_close(code, boost::beast::websocket::close_reason{cc});
}

}  // namespace clover2_http::http::transport
