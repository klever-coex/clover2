#pragma once

#include <clover2_http/http/endpoint/interface.hpp>

#include <boost/beast/core/detail/config.hpp>

#include <functional>
#include <type_traits>

namespace clover2_http::http::endpoint {

template <typename Req, typename Resp>
class adapter : public interface {
public:
    using handler =
        std::function<void(core::request_context, Req, reply<Resp>)>;

    explicit adapter(handler h)
        : m_handler(std::move(h)) {}

    void invoke(core::request_context ctx, http_request req,
                response_sender sender) override {
        try {
            Req request{};
            if (!req.body().empty()) {
                request = nlohmann::json::parse(req.body()).get<Req>();
            }

            m_handler(std::move(ctx), std::move(request),
                      reply<Resp>(std::move(sender)));
        } catch (const core::http_error& e) {
            sender(make_error_response(e.status(), e.message()));
        } catch (const nlohmann::json::exception& e) {
            sender(make_error_response(
                400, std::string("Invalid JSON: ") + e.what()));
        } catch (const std::exception& e) {
            sender(make_error_response(
                400, std::string("Bad request: ") + e.what()));
        }
    }

private:
    handler m_handler;
};

template <typename Resp>
class adapter<void, Resp> : public interface {
public:
    using handler = std::function<void(core::request_context, reply<Resp>)>;

    explicit adapter(handler h)
        : m_handler(std::move(h)) {}

    void invoke(core::request_context ctx, http_request /*req*/,
                response_sender sender) override {
        try {
            m_handler(std::move(ctx), reply<Resp>(std::move(sender)));
        } catch (const core::http_error& e) {
            sender(make_error_response(e.status(), e.message()));
        } catch (const std::exception& e) {
            sender(make_error_response(
                400, std::string("Bad request: ") + e.what()));
        }
    }

private:
    handler m_handler;
};

template <typename Req>
class adapter<Req, void> : public interface {
public:
    using handler =
        std::function<void(core::request_context, Req, reply<void>)>;

    explicit adapter(handler h)
        : m_handler(std::move(h)) {}

    void invoke(core::request_context ctx, http_request req,
                response_sender sender) override {
        try {
            Req request{};
            if (!req.body().empty()) {
                auto jv = nlohmann::json::parse(req.body());
                request = jv.get<Req>(jv);
            }

            m_handler(std::move(ctx), std::move(request),
                      reply<void>(std::move(sender)));
        } catch (const core::http_error& e) {
            sender(make_error_response(e.status(), e.message()));
        } catch (const nlohmann::json::exception& e) {
            sender(make_error_response(
                400, std::format("Invalid JSON: {}", e.what())));
        } catch (const std::exception& e) {
            sender(make_error_response(
                400, std::format("Bad request: {}", e.what())));
        }
    }

private:
    handler m_handler;
};

}  // namespace clover2_http::http::endpoint
