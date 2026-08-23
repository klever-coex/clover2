#include <clover2_http/http/endpoint/reply.hpp>

namespace clover2_http::http::endpoint {

namespace {

std::string error_body(const std::string& message) {
    nlohmann::json obj;
    obj["error"] = message;
    return obj.dump();
}

}  // namespace

void make_error_response(http_response& resp, int status,
                         const std::string& message) {
    resp.result(status);
    resp.set(boost::beast::http::field::content_type, "application/json");
    resp.body() = error_body(message);
    resp.prepare_payload();
}

reply_base::reply_base(response_sender sender)
    : m_sender(std::move(sender)) {
    m_response.version(11);
}

reply_base::reply_base(reply_base&& other) noexcept
    : m_sent(other.m_sent.load())
    , m_sender(std::move(other.m_sender))
    , m_response(std::move(other.m_response)) {
    other.m_sender = nullptr;
}

reply_base& reply_base::operator=(reply_base&& other) noexcept {
    if (this != &other) {
        m_sent.store(other.m_sent.load());
        m_sender = std::move(other.m_sender);
        other.m_sender = nullptr;
        m_response = std::move(other.m_response);
    }

    return *this;
}

reply_base::~reply_base() {
    if (!m_sent.load() && m_sender) {
        make_error_response(m_response, 500,
                            "Handler did not produce a response");

        m_sender(m_response);
    }
}

void reply_base::error(int status) {
    send_raw("", "", status);
}

void reply_base::error_json(int status, const std::string& message) {
    send_raw(error_body(message), "application/json", status);
}

std::string reply_base::header(std::string_view header) const {
    return m_response[header];
}

void reply_base::header(std::string_view header, std::string_view value) {
    m_response.set(header, value);
}

void reply_base::send_raw(std::string body, std::string_view content_type,
                          int status) {
    // Send-once guard must be the first statement: concurrent completions
    // of a deferred reply race here.
    if (m_sent.exchange(true)) return;

    m_response.result(status);
    if (!content_type.empty()) {
        m_response.set(boost::beast::http::field::content_type, content_type);
    }

    if (status != 204) {  // 204 must not carry a body / Content-Length
        m_response.body() = std::move(body);
        m_response.prepare_payload();
    }

    m_sender(std::move(m_response));
}

}  // namespace clover2_http::http::endpoint
