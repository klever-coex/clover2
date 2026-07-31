#pragma once

#include <clover2_http/serialization/json_traits.hpp>

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <string>

namespace clover2_http::endpoint {

inline constexpr unsigned kHttpVersion11 = 11;  // HTTP/1.1

using http_request =
    boost::beast::http::request<boost::beast::http::string_body>;
using http_response =
    boost::beast::http::response<boost::beast::http::string_body>;

using response_sender = std::function<void(http_response)>;

inline http_response make_ok_response(const std::string& body) {
    http_response res{boost::beast::http::status::ok, kHttpVersion11};
    res.set(boost::beast::http::field::content_type, "application/json");
    res.body() = body;
    res.prepare_payload();
    return res;
}

inline http_response make_error_response(int status,
                                         const std::string& message) {
    http_response res{static_cast<boost::beast::http::status>(status),
                      kHttpVersion11};
    res.set(boost::beast::http::field::content_type, "application/json");

    nlohmann::json obj;
    obj["error"] = message;
    res.body() = obj.dump();
    res.prepare_payload();
    return res;
}

template <typename T>
class reply {
public:
    explicit reply(response_sender sender)
        : m_sender(std::move(sender)) {}

    ~reply() {
        if (!m_sent && m_sender) {
            m_sender(
                make_error_response(500, "Handler did not produce a response"));
        }
    }

    reply(const reply&) = delete;
    reply& operator=(const reply&) = delete;

    reply(reply&&) = default;
    reply& operator=(reply&&) = default;

    void operator()(const T& resp, int status = 200) {
        if (m_sent) return;
        m_sent = true;

        auto body = serialization::impl::serialize(resp).dump();

        http_response response{static_cast<boost::beast::http::status>(status),
                               kHttpVersion11};
        response.set(boost::beast::http::field::content_type,
                     "application/json");
        response.body() = std::move(body);
        response.prepare_payload();

        m_sender(std::move(response));
    }

    void error(int status, const std::string& message) {
        if (m_sent) return;
        m_sent = true;
        m_sender(make_error_response(status, message));
    }

    response_sender release() {
        m_sent = true;
        return std::move(m_sender);
    }

private:
    response_sender m_sender;
    bool m_sent = false;
};

template <>
class reply<void> {
public:
    explicit reply(response_sender sender)
        : m_sender(std::move(sender)) {}

    ~reply() {
        if (!m_sent && m_sender) {
            m_sender(
                make_error_response(500, "Handler did not produce a response"));
        }
    }

    reply(const reply&) = delete;
    reply& operator=(const reply&) = delete;

    reply(reply&&) = default;
    reply& operator=(reply&&) = default;

    void done(int status = 200) {
        if (m_sent) return;
        m_sent = true;

        http_response response{static_cast<boost::beast::http::status>(status),
                               kHttpVersion11};
        response.set(boost::beast::http::field::content_type,
                     "application/json");
        response.body() = "{}";
        response.prepare_payload();

        m_sender(std::move(response));
    }

    void error(int status, const std::string& message) {
        if (m_sent) return;
        m_sent = true;
        m_sender(make_error_response(status, message));
    }

    response_sender release() {
        m_sent = true;
        return std::move(m_sender);
    }

private:
    response_sender m_sender;
    bool m_sent = false;
};

}  // namespace clover2_http::endpoint
