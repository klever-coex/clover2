#include <clover2/backend/server.hpp>
#include <clover2/backend/app.hpp>

namespace clover2::backend {

server::server(const rclcpp::NodeOptions& options)
    : Node("clover2_backend_server", options) {
    m_init_timer =
        this->create_wall_timer(std::chrono::milliseconds(0), [this]() {
            init_app();

            m_app->run(3000);

            m_init_timer->cancel();
        });

}

server::~server() {
    m_app->stop();
}

void server::init_app() {
    m_app = std::make_shared<app>();

    // add callbacks here
}

}  // namespace clover2::backend

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(clover2::backend::server)
