#pragma once

#include <clover2_http/http/core/request_context.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/websocket/stream.hpp>

#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace clover2_http::http::transport {

class base_ws_session : public std::enable_shared_from_this<base_ws_session> {
public:
    using text_handler =
        std::function<void(std::shared_ptr<base_ws_session>, std::string)>;
    using binary_handler = std::function<void(std::shared_ptr<base_ws_session>,
                                              std::vector<uint8_t>)>;
    using close_handler =
        std::function<void(std::shared_ptr<base_ws_session>, int)>;
    using connection_handler =
        std::function<void(std::shared_ptr<base_ws_session>)>;

    using strand_type =
        boost::asio::strand<boost::asio::io_context::executor_type>;

    explicit base_ws_session(boost::asio::ip::tcp::socket socket,
                             boost::asio::io_context& io);
    ~base_ws_session();

    void start(
        boost::beast::http::request<boost::beast::http::string_body> request,
        core::request_context ctx, connection_handler handler);

    void on_text(text_handler handler);
    void on_binary(binary_handler handler);
    void on_close(close_handler handler);

    void start_reading();

    void write_text(std::string data);
    void write_binary(std::vector<uint8_t> data);
    void write_binary(const uint8_t* data, size_t size);

    void close(int code = 1000, const std::string& reason = "");
    void ping(std::string payload = "");

    bool is_open() const;
    core::request_context& context();

private:
    struct queued_message {
        std::string data;
        bool is_binary;
    };

    void write_raw(std::string data, bool binary);
    void do_read();
    void on_read(boost::system::error_code ec);
    void dispatch_binary(std::string data);
    void dispatch_text(std::string data);
    void do_write();
    void do_close_ws(boost::beast::websocket::close_code code =
                         boost::beast::websocket::close_code::normal);

    boost::beast::websocket::stream<boost::asio::ip::tcp::socket> m_ws;
    strand_type m_strand;
    boost::beast::flat_buffer m_buffer;
    core::request_context m_ctx;

    text_handler m_text_handler;
    binary_handler m_binary_handler;
    close_handler m_close_handler;

    std::deque<queued_message> m_write_queue;
    bool m_writing = false;
    bool m_closed = false;
};

}  // namespace clover2_http::http::transport
