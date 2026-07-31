#pragma once

#include <clover2_http/core/logger.hpp>

#include <memory>
#include <string>

namespace clover2_http::core {

class std_logger : public logger {
public:
    explicit std_logger(const std::string& name)
        : m_name(name) {}

    void debug(const std::string& msg) override {
        std::cout << "[ DEBUG ]" << m_name << msg << std::endl;
    }

    void info(const std::string& msg) override {
        std::cout << "[ INFO  ]" << m_name << msg << std::endl;
    }

    void warn(const std::string& msg) override {
        std::cout << "[ WARN  ]" << m_name << msg << std::endl;
    }

    void error(const std::string& msg) override {
        std::cout << "[ ERROR ]" << m_name << msg << std::endl;
    }

private:
    std::string m_name;
};

}  // namespace clover2_http::core
