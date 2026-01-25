#pragma once

#include <thread>

namespace clover2::backend {

class app {
public:
    app();
    virtual ~app();

    void run(int port);
    void stop();
private:
    std::thread m_app_thread;
};

}  // namespace clover2::backend