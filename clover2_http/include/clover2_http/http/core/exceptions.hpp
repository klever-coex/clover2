#pragma once

#include <exception>
#include <format>
#include <string>

namespace clover2_http::http::core {

class http_error : public std::exception {
public:
    http_error(int status, std::string message) noexcept
        : m_status(status)
        , m_message(std::move(message)) {}

    int status() const noexcept { return m_status; }
    const char* what() const noexcept override { return m_message.c_str(); }
    const std::string& message() const noexcept { return m_message; }

private:
    int m_status;
    std::string m_message;
};

class routing_error : public std::exception {
public:
    template <typename... Args>
    routing_error(std::format_string<Args...> fmt, Args&&... args) noexcept
        : m_message(std::format(fmt, std::forward<Args>(args)...)) {}

    const char* what() const noexcept override { return m_message.c_str(); }
    const std::string& message() const noexcept { return m_message; }

private:
    std::string m_message;
};

class parsing_error : public std::exception {
public:
    template <typename... Args>
    parsing_error(std::format_string<Args...> fmt, Args&&... args) noexcept
        : m_message(std::format(fmt, std::forward<Args>(args)...)) {}

    const char* what() const noexcept override { return m_message.c_str(); }
    const std::string& message() const noexcept { return m_message; }

private:
    std::string m_message;
};

}  // namespace clover2_http::http::core
