#pragma once

#include <clover2_http/core/logger.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/detail/config.hpp>

#include <memory>
#include <string>

namespace clover2_http::routing {
class router;
}

namespace clover2_http::transport {

class listener : public std::enable_shared_from_this<listener> {
public:
    listener(boost::asio::io_context& io,
             const boost::asio::ip::tcp::endpoint& endpoint,
             routing::router& router,
             std::shared_ptr<clover2_http::core::logger> log);
    ~listener();

    void start();

private:
    void do_accept();

    boost::asio::io_context& m_io;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::ip::tcp::socket m_socket;
    routing::router& m_router;
    std::shared_ptr<clover2_http::core::logger> m_logger;
};

}  // namespace clover2_http::transport
