#pragma once

#include <cstdint>  // must precede beast: field.hpp uses std::uint32_t

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <string>

namespace clover2_http::http::endpoint {

using http_request =
    boost::beast::http::request<boost::beast::http::string_body>;
using http_response =
    boost::beast::http::response<boost::beast::http::string_body>;

using response_sender = std::function<void(http_response)>;

http_response make_ok_response(const std::string& body);
http_response make_error_response(int status, const std::string& message);

class reply_base {
public:
    explicit reply_base(response_sender sender)
        : m_sender(std::move(sender)) {}

    ~reply_base() {
        if (!m_sent && m_sender) {
            m_sender(
                make_error_response(500, "Handler did not produce a response"));
        }
    }

    reply_base(const reply_base&) = delete;
    reply_base& operator=(const reply_base&) = delete;

    reply_base(reply_base&&) = default;
    reply_base& operator=(reply_base&&) = default;

    void error(int status, const std::string& message) {
        if (m_sent) return;

        m_sent = true;
        m_sender(make_error_response(status, message));
    }

    response_sender release() {
        m_sent = true;
        return std::move(m_sender);
    }

protected:
    bool m_sent = false;
    response_sender m_sender;
};

template <typename T>
class reply : public reply_base {
public:
    explicit reply(response_sender sender)
        : reply_base(std::move(sender)) {}

    void operator()(const T& resp, int status = 200) {
        if (m_sent) return;
        m_sent = true;

        nlohmann::json jv(resp);
        std::string body = jv.dump();

        http_response response{static_cast<boost::beast::http::status>(status),
                               11};
        response.set(boost::beast::http::field::content_type,
                     "application/json");
        response.body() = std::move(body);
        response.prepare_payload();

        m_sender(std::move(response));
    }
};

template <>
class reply<void> : public reply_base {
public:
    explicit reply(response_sender sender)
        : reply_base(std::move(sender)) {}

    void done(int status = 200) {
        if (m_sent) return;
        m_sent = true;

        http_response response{static_cast<boost::beast::http::status>(status),
                               11};
        response.set(boost::beast::http::field::content_type,
                     "application/json");
        response.body() = "{}";
        response.prepare_payload();

        m_sender(std::move(response));
    }
};

}  // namespace clover2_http::http::endpoint
