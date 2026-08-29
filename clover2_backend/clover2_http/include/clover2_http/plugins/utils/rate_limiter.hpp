#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>

namespace clover2_http_plugins::utils {

class rate_limiter {
public:
    explicit rate_limiter(double bytes_per_second)
        : m_bytes_per_second(bytes_per_second > 0.0 ? bytes_per_second : 0.0)
        , m_burst(bytes_per_second > 0.0
                      ? std::max(bytes_per_second, 64.0 * 1024.0)
                      : 0.0)
        , m_tokens(bytes_per_second > 0.0
                       ? std::max(bytes_per_second, 64.0 * 1024.0)
                       : 0.0)
        , m_last(std::chrono::steady_clock::now()) {}

    bool allow(size_t bytes) {
        if (m_bytes_per_second <= 0.0) {
            return true;
        }

        const auto now = std::chrono::steady_clock::now();
        const double elapsed =
            std::chrono::duration<double>(now - m_last).count();
        m_last = now;

        m_tokens =
            std::min(m_burst, m_tokens + elapsed * m_bytes_per_second);

        if (static_cast<double>(bytes) > m_burst) {
            if (m_tokens >= m_burst) {
                m_tokens = 0.0;
                return true;
            }
            return false;
        }

        if (static_cast<double>(bytes) > m_tokens) {
            return false;
        }

        m_tokens -= static_cast<double>(bytes);

        return true;
    }

private:
    double m_bytes_per_second;
    double m_burst;
    double m_tokens;
    std::chrono::steady_clock::time_point m_last;
};

}  // namespace clover2_http_plugins::utils
