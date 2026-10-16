#include <clover2_http/http/core/logger.hpp>

#include <iostream>

namespace clover2_http::http::core {

simple_logger::simple_logger(const std::string& name)
    : m_name(name) {}

void simple_logger::log(logger::level lvl, std::string msg) {
    static const char* labels[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    auto idx = static_cast<int>(lvl);
    std::cout << std::format(
        "[{:>6}] {}: {}\n", labels[idx >= 0 && idx < 4 ? idx : 0], m_name, msg);
}

}  // namespace clover2_http::http::core
