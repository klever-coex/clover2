#pragma once

// clover2
#include <clover2_http/http/core/logger.hpp>
#include <clover2_http/http/core/settings.hpp>

// boost
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/detail/config.hpp>

// STL
#include <memory>

namespace clover2_http::http::routing {
class router;
}

namespace clover2_http::http::transport {

class listener : public std::enable_shared_from_this<listener> {
public:
    listener(boost::asio::io_context& io,
             const boost::asio::ip::tcp::endpoint& endpoint,
             routing::router& router,
             std::shared_ptr<clover2_http::http::core::logger> log,
             const core::settings& settings);
    ~listener();

    void start();
    void stop();

private:
    void do_accept();

    const core::settings& m_settings;

    boost::asio::io_context& m_io;
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::steady_timer m_retry_timer;
    routing::router& m_router;
    std::shared_ptr<clover2_http::http::core::logger> m_logger;
};

}  // namespace clover2_http::http::transport
