// clover2
#include <clover2_http/http/transport/http_session.hpp>
#include <clover2_http/http/transport/listener.hpp>

// boost
#include <boost/asio/bind_executor.hpp>

namespace clover2_http::http::transport {

listener::listener(boost::asio::io_context& io,
                   const boost::asio::ip::tcp::endpoint& endpoint,
                   routing::router& router,
                   std::shared_ptr<clover2_http::http::core::logger> log,
                   const core::settings& settings)
    : m_settings(settings)
    , m_io(io)
    , m_acceptor(io)
    , m_retry_timer(io)
    , m_router(router)
    , m_logger(std::move(log)) {
    boost::system::error_code ec;

    ec = m_acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        m_logger->error("Acceptor open: {}", ec.message());
        throw std::runtime_error("Acceptor open: " + ec.message());
    }

    ec = m_acceptor.set_option(
        boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    if (ec) {
        m_logger->error("Acceptor set_option: {}", ec.message());
        throw std::runtime_error("Acceptor set_option: " + ec.message());
    }

    ec = m_acceptor.bind(endpoint, ec);
    if (ec) {
        m_logger->error("Acceptor bind: {}", ec.message());
        throw std::runtime_error("Acceptor bind: " + ec.message());
    }

    ec =
        m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        m_logger->error("Acceptor listen: {}", ec.message());
        throw std::runtime_error("Acceptor listen: " + ec.message());
    }
}

listener::~listener() {
    m_retry_timer.cancel();
    m_acceptor.close();
}

void listener::start() { do_accept(); }

void listener::stop() {
    m_retry_timer.cancel();
    m_acceptor.close();
}

void listener::do_accept() {
    auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_io);

    m_acceptor.async_accept(*socket, [self = shared_from_this(),
                                      socket](boost::system::error_code ec) {
        if (ec) {
            if (ec == boost::asio::error::operation_aborted) {
                return;
            }

            self->m_logger->warn("Accept error: {}", ec.message());
            if (!self->m_acceptor.is_open()) {
                return;
            }

            self->m_retry_timer.expires_after(std::chrono::milliseconds(100));
            self->m_retry_timer.async_wait(
                [self](boost::system::error_code ec) {
                    if (!ec) {
                        self->do_accept();
                    }
                });
            return;
        }

        auto session = std::make_shared<http_session>(
            std::move(*socket), self->m_router, self->m_io, self->m_logger,
            self->m_settings);

        session->start();

        self->do_accept();
    });
}

}  // namespace clover2_http::http::transport
