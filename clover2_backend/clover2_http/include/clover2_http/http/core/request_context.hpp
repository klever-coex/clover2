#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/url.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_http::http::core {

// Per-request context handed to handlers. Header keys are lowercase;
// path/query values are percent-decoded.
struct request_context {
    request_context() = default;

    explicit request_context(boost::urls::url_view parsed_url)
        : url(parsed_url) {
        for (const auto& q : parsed_url.params()) {
            query_params.push_back(
                boost::urls::param{q.key, q.value, q.has_value});
        }
    }

    // Typed path parameter access. Throws std::out_of_range if the route
    // does not declare the name; conversion failures throw
    // nlohmann::json::exception (surfaced as 400 by the adapter).
    template <typename T>
    T param(const std::string& name) const {
        const auto& s = path_params.at(name);
        if constexpr (std::is_same_v<T, std::string>) {
            return s;
        } else {
            return nlohmann::json::parse(s).get<T>();
        }
    }

    // Typed query parameter access; the last occurrence wins, returns
    // fallback if the parameter is absent.
    template <typename T>
    T query(const std::string& name, const T& fallback = T{}) const {
        for (auto it = query_params.rbegin(); it != query_params.rend(); ++it) {
            if (it->key == name) {
                if constexpr (std::is_same_v<T, std::string>) {
                    return it->value;
                } else {
                    return nlohmann::json::parse(it->value).get<T>();
                }
            }
        }
        return fallback;
    }

    boost::asio::ip::tcp::endpoint remote_endpoint;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> path_params;

    // Owned copy: a url_view would reference the request buffer, which is
    // reused for the next request on keep-alive.
    boost::urls::url url;

    // Decoded pairs, order and duplicates preserved.
    std::vector<boost::urls::param> query_params;
};

}  // namespace clover2_http::http::core
