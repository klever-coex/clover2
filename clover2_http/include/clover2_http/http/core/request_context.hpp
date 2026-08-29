#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/url.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace clover2_http::http::core {

struct request_context {
    request_context() = default;

    explicit request_context(boost::urls::url_view parsed_url)
        : url(parsed_url) {
        for (const auto& q : parsed_url.params()) {
            query_params.push_back(
                boost::urls::param{q.key, q.value, q.has_value});
        }
    }

    template <typename T>
    T param(const std::string& name) const {
        const auto& s = path_params.at(name);
        if constexpr (std::is_same_v<T, std::string>) {
            return s;
        } else {
            return nlohmann::json::parse(s).get<T>();
        }
    }

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

    boost::urls::url url;

    std::vector<boost::urls::param> query_params;
};

}  // namespace clover2_http::http::core
