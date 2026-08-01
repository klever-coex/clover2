#include <clover2_http/transport/http_session.hpp>
#include <clover2_http/transport/listener.hpp>

#include <boost/asio/bind_executor.hpp>

namespace clover2_http::transport {

listener::listener(boost::asio::io_context& io,
                   const boost::asio::ip::tcp::endpoint& endpoint,
                   routing::router& router,
                   std::shared_ptr<clover2_http::core::logger> log)
    : m_io(io)
    , m_acceptor(io)
    , m_socket(io)
    , m_router(router)
    , m_logger(std::move(log)) {
    boost::system::error_code ec;

    m_acceptor.open(endpoint.protocol(), ec);
    if (ec) {
        m_logger->error("Acceptor open: {}", ec.message());
        return;
    }

    m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true),
                          ec);
    if (ec) {
        m_logger->error("Acceptor set_option: {}", ec.message());
        return;
    }

    m_acceptor.bind(endpoint, ec);
    if (ec) {
        m_logger->error("Acceptor bind: {}", ec.message());
        return;
    }

    m_acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) {
        m_logger->error("Acceptor listen: {}", ec.message());
        return;
    }
}

listener::~listener() {
    boost::system::error_code ec;
    m_acceptor.close(ec);
}

void listener::start() { do_accept(); }

void listener::do_accept() {
    m_acceptor.async_accept(
        m_socket, [self = shared_from_this()](boost::system::error_code ec) {
            if (ec) {
                self->m_logger->warn("Accept error: {}", ec.message());
                self->do_accept();
                return;
            }

            auto session = std::make_shared<http_session>(
                std::move(self->m_socket), self->m_router, self->m_io,
                self->m_logger);

            session->start();

            self->do_accept();
        });
}

}  // namespace clover2_http::transport
