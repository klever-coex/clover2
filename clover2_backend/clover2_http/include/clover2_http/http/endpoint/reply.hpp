#pragma once

// clang-format off

// STL
#include <cstdint>  // must precede beast: field.hpp uses std::uint32_t

// boost
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/string_body.hpp>

// JSON
#include <nlohmann/json.hpp>

// STL again
#include <atomic>
#include <concepts>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>

// clang-format on

namespace clover2_http::http::endpoint {

template <typename T>
concept json_serializable = std::constructible_from<nlohmann::json, const T&>;

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

    // Only move without copy. The move is hand-written because
    // std::atomic_bool is not movable; the moved-from sender is reset so
    // that a moved-from reply never sends the 500 fallback.
    reply_base(const reply_base&) = delete;
    reply_base& operator=(const reply_base&) = delete;

    reply_base(reply_base&& other) noexcept;
    reply_base& operator=(reply_base&& other) noexcept;

    void error(int status);
    void error_json(int status, const std::string& message);

    std::string header(std::string_view header) const;
    void header(std::string_view header, std::string_view value);

protected:
    // Atomic: a deferred reply may be completed from another thread.
    std::atomic_bool m_sent{false};
    response_sender m_sender;
    http_response m_response;

    void send_raw(std::string body, std::string_view content_type,
                  int status);
};

template <typename T>
class reply : public reply_base {
public:
    explicit reply(response_sender sender)
        : reply_base(std::move(sender)) {}

    explicit reply(reply_base&& base)
        : reply_base(std::move(base)) {}

    void operator()(const T& resp, int status = 200)
        requires json_serializable<T>
    {
        nlohmann::json jv(resp);
        send_raw(jv.dump(), "application/json", status);
    }

    void operator()(std::string_view body, std::string_view content_type,
                    int status = 200) {
        send_raw(std::string(body), content_type, status);
    }

    void operator()(std::span<const std::uint8_t> data,
                    std::string_view content_type, int status = 200) {
        std::string body(reinterpret_cast<const char*>(data.data()),
                         data.size());
        send_raw(std::move(body), content_type, status);
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
        send_raw("", "text/plain", status);
    }
};

// A shared handle to a reply that outlives the handler. Handlers receive
// it by value; a copy captured into a callback keeps the reply alive, and
// the response may be sent from any thread after the handler returned
// (the underlying send is serialized by the session strand, and the
// atomic send-once guard makes concurrent completions safe). If the last
// copy is destroyed without a response, the reply destructor sends the
// usual 500 fallback.
template <typename T>
class deferred_reply {
public:
    explicit deferred_reply(std::shared_ptr<reply<T>> inner)
        : m_inner(std::move(inner)) {}

    void operator()(const T& resp, int status = 200) const
        requires json_serializable<T>
    {
        m_inner->operator()(resp, status);
    }

    void error(int status) const { m_inner->error(status); }

    void error_json(int status, const std::string& message) const {
        m_inner->error_json(status, message);
    }

    std::string header(std::string_view name) const {
        return m_inner->header(name);
    }

    void header(std::string_view name, std::string_view value) const {
        m_inner->header(name, value);
    }

private:
    std::shared_ptr<reply<T>> m_inner;
};

}  // namespace clover2_http::http::endpoint
