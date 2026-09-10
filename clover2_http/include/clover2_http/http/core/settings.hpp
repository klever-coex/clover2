#pragma once

// STL
#include <chrono>

namespace clover2_http::http::core {

class settings {
public:
    settings() = default;
    ~settings() = default;

    settings& max_body_size(size_t s) {
        m_max_body_size = s;
        return *this;
    }

    size_t max_body_size() const {  //
        return m_max_body_size;
    }

    settings& websocket_timeout(std::chrono::seconds t) {
        m_websocket_timeout = std::move(t);
        return *this;
    }

    std::chrono::seconds websocket_timeout() const {
        return m_websocket_timeout;
    }

    settings& http_timeout(std::chrono::seconds t) {
        m_http_timeout = std::move(t);
        return *this;
    }

    std::chrono::seconds http_timeout() const {  //
        return m_http_timeout;
    }

private:
    size_t m_max_body_size{10 * 1024 * 1024};
    std::chrono::seconds m_http_timeout{30};
    std::chrono::seconds m_websocket_timeout{60};
};

}  // namespace clover2_http::http::core
