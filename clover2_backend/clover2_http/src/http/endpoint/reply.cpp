#include <clover2_http/http/endpoint/reply.hpp>

namespace clover2_http::http::endpoint {

http_response make_ok_response(const std::string& body) {
    http_response res{boost::beast::http::status::ok, 11};
    res.set(boost::beast::http::field::content_type, "application/json");
    res.body() = body;
    res.prepare_payload();
    return res;
}

http_response make_error_response(int status, const std::string& message) {
    http_response res{static_cast<boost::beast::http::status>(status), 11};
    res.set(boost::beast::http::field::content_type, "application/json");

    nlohmann::json obj;
    obj["error"] = message;
    res.body() = obj.dump();
    res.prepare_payload();
    return res;
}

}  // namespace clover2_http::http::endpoint
