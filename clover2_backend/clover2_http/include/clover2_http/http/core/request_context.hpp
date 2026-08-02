#pragma once

#include <clover2_http/http/core/dynamic_params.hpp>

#include <boost/asio/ip/tcp.hpp>

#include <string>
#include <unordered_map>

namespace clover2_http::http::core {

struct request_context {
    boost::asio::ip::tcp::endpoint remote_endpoint;
    std::unordered_map<std::string, std::string> headers;
    dynamic_params path_params;
    dynamic_params query_params;
};

}  // namespace clover2_http::http::core
