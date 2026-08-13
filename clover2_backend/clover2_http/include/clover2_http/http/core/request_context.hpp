#pragma once

#include <boost/asio/ip/tcp.hpp>

#include <string>
#include <unordered_map>

namespace clover2_http::http::core {

struct request_context {
    boost::asio::ip::tcp::endpoint remote_endpoint;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> path_params;
    std::unordered_map<std::string, std::string> query_params;
};

}  // namespace clover2_http::http::core
