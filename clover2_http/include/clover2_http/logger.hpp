#pragma once

#include <clover2_http/http/core/logger.hpp>

#include <rclcpp/logger.hpp>
#include <rclcpp/logging.hpp>

#include <utility>

namespace clover2_http {

class logger : public clover2_http::http::core::logger {
public:
    explicit logger(rclcpp::Logger logger)
        : m_logger(std::move(logger)) {}

protected:
    void log(clover2_http::http::core::logger::level lvl,
             std::string msg) override final {
        switch (lvl) {
            case clover2_http::http::core::logger::level::debug:
                RCLCPP_DEBUG(m_logger, "%s", msg.c_str());
                break;
            case clover2_http::http::core::logger::level::info:
                RCLCPP_INFO(m_logger, "%s", msg.c_str());
                break;
            case clover2_http::http::core::logger::level::warn:
                RCLCPP_WARN(m_logger, "%s", msg.c_str());
                break;
            case clover2_http::http::core::logger::level::error:
                RCLCPP_ERROR(m_logger, "%s", msg.c_str());
                break;
        }
    }

private:
    rclcpp::Logger m_logger;
};

}  // namespace clover2_http
