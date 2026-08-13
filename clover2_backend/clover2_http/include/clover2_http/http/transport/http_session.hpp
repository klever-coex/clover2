#pragma once

#include <clover2_http/http/core/logger.hpp>
#include <clover2_http/http/core/request_context.hpp>

#include <boost/url.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/detail/config.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

#include <memory>
#include <unordered_map>

namespace clover2_http::http::routing {
class router;
}

namespace clover2_http::http::transport {

struct parsed_target {
    std::string path;
    std::unordered_map<std::string, std::string> query_params;
};

parsed_target parse_target(const std::string& target);

class http_session : public std::enable_shared_from_this<http_session> {
public:
    http_session(boost::asio::ip::tcp::socket socket, routing::router& router,
                 boost::asio::io_context& io,
                 std::shared_ptr<clover2_http::http::core::logger> log);
    ~http_session();

    void start();

    boost::asio::ip::tcp::socket& socket() { return m_socket; }

private:
    void do_read();
    void on_read(boost::beast::error_code ec, std::size_t bytes);
    void handle_request();
    void send_error(int status, const std::string& message);
    void do_write(
        boost::beast::http::response<boost::beast::http::string_body> response);
    void on_write(boost::beast::error_code ec, std::size_t bytes, bool close);
    void do_close();
    core::request_context make_context(const parsed_target& pt);
    core::request_context make_context(boost::urls::url_view url);

    boost::asio::ip::tcp::socket m_socket;

    using strand_type =
        boost::asio::strand<boost::asio::io_context::executor_type>;
    strand_type m_strand;

    bool m_close = false;
    bool m_keep_alive = false;
    int m_version = 11;  // HTTP/1.1

    boost::beast::flat_buffer m_buffer;
    boost::beast::http::request<boost::beast::http::string_body> m_request;
    boost::beast::http::response<boost::beast::http::string_body> m_response;

    routing::router& m_router;
    boost::asio::steady_timer m_timer;
    std::shared_ptr<clover2_http::http::core::logger> m_logger;
};

}  // namespace clover2_http::http::transport
