#pragma once

// clover2
#include "clover2_http/http/core/logger.hpp"
#include <clover2_http/http/endpoint/interface.hpp>

// boost
#include <boost/beast/core/detail/config.hpp>

// STL
#include <format>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

namespace clover2_http::http::endpoint {

template <typename Req, typename Resp>
class adapter : public interface {
public:
    using handler =
        std::function<void(core::request_context, Req, deferred_reply<Resp>)>;

    explicit adapter(handler h, std::shared_ptr<core::logger> logger)
        : m_handler(std::move(h))
        , m_logger(std::move(logger)) {}

    void invoke(core::request_context& ctx, http_request& req,
                std::shared_ptr<reply_base> base) override {
        auto typed = std::make_shared<reply<Resp>>(std::move(*base));

        try {
            Req request{};
            if (!req.body().empty()) {
                request = nlohmann::json::parse(req.body()).get<Req>();
            }

            m_handler(std::move(ctx),      //
                      std::move(request),  //
                      deferred_reply<Resp>(typed));

        } catch (const core::http_error& e) {
            typed->error_json(e.status(), e.message());
        } catch (const nlohmann::json::exception& e) {
            typed->error_json(400, std::format("Invalid JSON: {}", e.what()));
        } catch (const std::invalid_argument& e) {
            typed->error_json(400,
                              std::format("Invalid argument: {}", e.what()));
        } catch (const std::exception& e) {
            if (m_logger) {
                m_logger->error("Internal Server Error: {}", e.what());
            }

            typed->error_json(500, "Internal Server Error");
        }
    }

private:
    handler m_handler;
    std::shared_ptr<core::logger> m_logger;
};

template <typename Resp>
class adapter<void, Resp> : public interface {
public:
    using handler =
        std::function<void(core::request_context, deferred_reply<Resp>)>;

    explicit adapter(handler h, std::shared_ptr<core::logger> logger)
        : m_handler(std::move(h))
        , m_logger(std::move(logger)) {}

    void invoke(core::request_context& ctx, http_request& /*req*/,
                std::shared_ptr<reply_base> base) override {
        auto typed = std::make_shared<reply<Resp>>(std::move(*base));

        try {
            m_handler(std::move(ctx),  //
                      deferred_reply<Resp>(typed));

        } catch (const core::http_error& e) {
            typed->error_json(e.status(), e.message());
        } catch (const std::exception& e) {
            if (m_logger) {
                m_logger->error("Internal Server Error: {}", e.what());
            }

            typed->error_json(500, "Internal Server Error");
        }
    }

private:
    handler m_handler;
    std::shared_ptr<core::logger> m_logger;
};

template <typename Req>
class adapter<Req, void> : public interface {
public:
    using handler =
        std::function<void(core::request_context, Req, deferred_reply<void>)>;

    explicit adapter(handler h, std::shared_ptr<core::logger> logger)
        : m_handler(std::move(h))
        , m_logger(std::move(logger)) {}

    void invoke(core::request_context& ctx, http_request& req,
                std::shared_ptr<reply_base> base) override {
        auto typed = std::make_shared<reply<void>>(std::move(*base));

        try {
            Req request{};

            if (!req.body().empty()) {
                auto jv = nlohmann::json::parse(req.body());
                request = jv.get<Req>();
            }

            m_handler(std::move(ctx),      //
                      std::move(request),  //
                      deferred_reply<void>(typed));

        } catch (const core::http_error& e) {
            typed->error_json(e.status(), e.message());
        } catch (const nlohmann::json::exception& e) {
            typed->error_json(400, std::format("Invalid JSON: {}", e.what()));
        } catch (const std::invalid_argument& e) {
            typed->error_json(400,
                              std::format("Invalid argument: {}", e.what()));
        } catch (const std::exception& e) {
            if (m_logger) {
                m_logger->error("Internal Server Error: {}", e.what());
            }

            typed->error_json(500, "Internal Server Error");
        }
    }

private:
    handler m_handler;
    std::shared_ptr<core::logger> m_logger;
};

}  // namespace clover2_http::http::endpoint
