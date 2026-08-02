#pragma once

#include <exception>
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
    routing_error(std::string message) noexcept
        : m_message(std::move(message)) {}

    const char* what() const noexcept override { return m_message.c_str(); }
    const std::string& message() const noexcept { return m_message; }

private:
    std::string m_message;
};

class parsing_error : public std::exception {
public:
    parsing_error(std::string message) noexcept
        : m_message(std::move(message)) {}

    const char* what() const noexcept override { return m_message.c_str(); }
    const std::string& message() const noexcept { return m_message; }

private:
    std::string m_message;
};

}  // namespace clover2_http::http::core
