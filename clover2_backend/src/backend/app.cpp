#include <clover2/backend/app.hpp>
#include <drogon/drogon.h>
#include <drogon/HttpAppFramework.h>

namespace clover2::backend {

app::app() {}

app::~app() { 
    stop(); //
}

void app::run(int port) {
    m_app_thread = std::thread([port]() {
        drogon::app().disableSigtermHandling();
        drogon::app().addListener("0.0.0.0", port);
        drogon::app().run();
    });
}

void app::stop() {
    drogon::app().quit();
    if (m_app_thread.joinable()) {
        m_app_thread.join();
    }
}

}  // namespace clover2::backend
