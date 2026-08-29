#pragma once

#include <cstdint>
#include <format>
#include <string>

namespace clover2_http::http::core {

class logger {
public:
    enum class level { debug, info, warn, error };

    virtual ~logger() = default;

    template <typename... Args>
    void debug(std::format_string<Args...> fmt, Args&&... args) {
        log(level::debug, std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void info(std::format_string<Args...> fmt, Args&&... args) {
        log(level::info, std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void warn(std::format_string<Args...> fmt, Args&&... args) {
        log(level::warn, std::format(fmt, std::forward<Args>(args)...));
    }

    template <typename... Args>
    void error(std::format_string<Args...> fmt, Args&&... args) {
        log(level::error, std::format(fmt, std::forward<Args>(args)...));
    }

protected:
    virtual void log(level lvl, std::string msg) = 0;
};

class simple_logger : public logger {
public:
    explicit simple_logger(const std::string& name);

protected:
    void log(logger::level lvl, std::string msg) override final;

    std::string m_name;
};

}  // namespace clover2_http::http::core
