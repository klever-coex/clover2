#include <clover2_http/http/endpoint/reply.hpp>

namespace clover2_http::http::endpoint {

void make_error_response(http_response& resp, int status,
                         const std::string& message) {
    resp.result(status);
    resp.set(boost::beast::http::field::content_type, "application/json");

    nlohmann::json obj;
    obj["error"] = message;
    resp.body() = obj.dump();
    resp.prepare_payload();
}

reply_base::reply_base(response_sender sender)
    : m_sender(std::move(sender)) {
    m_response.version(11);
}

reply_base::~reply_base() {
    if (!m_sent && m_sender) {
        make_error_response(m_response, 500,
                            "Handler did not produce a response");

        m_sender(m_response);
    }
}

void reply_base::error(int status) {
    if (m_sent) return;

    m_sent = true;
    m_response.result(status);
    if (status != 204) {               // 204 must not carry Content-Length
        m_response.prepare_payload();  // sets necessary headers
    }

    m_sender(m_response);
}

void reply_base::error_json(int status, const std::string& message) {
    if (m_sent) return;

    m_sent = true;

    make_error_response(m_response, status, message);
    m_sender(m_response);
}

std::string reply_base::get_header(std::string_view header) const {
    return m_response[header];
}

void reply_base::set_header(std::string_view header, std::string_view value) {
    m_response.set(header, value);
}

}  // namespace clover2_http::http::endpoint
