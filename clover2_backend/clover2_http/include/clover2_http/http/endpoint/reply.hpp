#pragma once

// clang-format off

#include <cstdint>  // must precede beast: field.hpp uses std::uint32_t

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>
#include <nlohmann/json.hpp>

// STL
#include <functional>
#include <string>

// clang-format on

namespace clover2_http::http::endpoint {

using http_request =
    boost::beast::http::request<boost::beast::http::string_body>;
using http_response =
    boost::beast::http::response<boost::beast::http::string_body>;

using response_sender = std::function<void(http_response)>;

void make_error_response(http_response& resp, int status,
                         const std::string& message);

class reply_base {
public:
    explicit reply_base(response_sender sender);
    ~reply_base();

    // Only move without copy
    reply_base(const reply_base&) = delete;
    reply_base& operator=(const reply_base&) = delete;

    reply_base(reply_base&&) = default;
    reply_base& operator=(reply_base&&) = default;

    void error(int status);
    void error_json(int status, const std::string& message);

    std::string get_header(std::string_view header) const;
    void set_header(std::string_view header, std::string_view value);

protected:
    bool m_sent = false;
    response_sender m_sender;
    http_response m_response;
};

template <typename T>
class reply : public reply_base {
public:
    explicit reply(response_sender sender)
        : reply_base(std::move(sender)) {}

    explicit reply(reply_base&& base)
        : reply_base(std::move(base)) {}

    void operator()(const T& resp, int status = 200) {
        if (m_sent) return;
        m_sent = true;

        nlohmann::json jv(resp);
        std::string body = jv.dump();

        m_response.result(status);
        m_response.set(boost::beast::http::field::content_type,
                       "application/json");
        m_response.body() = std::move(body);
        m_response.prepare_payload();

        m_sender(std::move(m_response));
    }
};

template <>
class reply<void> : public reply_base {
public:
    explicit reply(response_sender sender)
        : reply_base(std::move(sender)) {}

    explicit reply(reply_base&& base)
        : reply_base(std::move(base)) {}

    void done(int status = 200) {
        if (m_sent) return;
        m_sent = true;

        m_response.result(status);
        m_response.set(boost::beast::http::field::content_type,
                       "application/json");
        m_response.body() = "{}";
        m_response.prepare_payload();

        m_sender(std::move(m_response));
    }
};

}  // namespace clover2_http::http::endpoint
