#pragma once

#include <clover2_http/http/core/exceptions.hpp>
#include <clover2_http/http/routing/utils.hpp>

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace clover2_http::http::routing {

template <typename HandlerT>
class trie {
public:
    struct node {
        using UniquePtr = std::unique_ptr<node>;

        bool is_end = false;

        UniquePtr parameter{nullptr};
        std::string parameter_name;

        UniquePtr catch_all{nullptr};
        std::string catch_all_name;

        std::unique_ptr<HandlerT> handler{nullptr};

        std::unordered_map<std::string, node::UniquePtr> children;
    };

    struct match_result {
        HandlerT* handler;
        std::unordered_map<std::string, std::string> params;
    };

    trie()
        : root(std::make_unique<node>()) {}

    void insert(const std::vector<std::string>& segments,
                const std::string_view route,
                std::unique_ptr<HandlerT> handler) {
        auto cur = root.get();

        for (const auto& seg : segments) {
            if (!seg.empty() && !is_token_valid(seg)) {
                throw clover2_http::http::core::routing_error(
                    "Invalid path token {}.", seg);
            }

            if (is_parameter(seg)) {
                cur = fill_parameter(seg, cur);
            } else if (is_catch_all(seg)) {
                if (&seg != &segments.back()) {
                    throw clover2_http::http::core::routing_error(
                        "Catch all should be the last segment.");
                }

                cur = fill_catch_all(seg, cur);
                break;
            } else {
                auto it = cur->children.find(seg);
                if (it == cur->children.end()) {
                    it = cur->children.emplace(seg, std::make_unique<node>())
                             .first;
                }

                cur = it->second.get();
            }
        }

        if (cur->is_end) {
            throw clover2_http::http::core::routing_error(
                "Route {} already exists", route);
        }

        cur->is_end = true;
        cur->handler = std::move(handler);
    }

    std::optional<match_result> search(
        const std::vector<std::string>& segments) const {
        auto cur = root.get();

        match_result result;

        for (size_t i = 0; i < segments.size(); ++i) {
            const auto& seg = segments[i];

            auto it = cur->children.find(seg);
            if (it != cur->children.end()) {
                cur = it->second.get();
                continue;
            }

            if (cur->parameter) {
                result.params[cur->parameter_name] = seg;
                cur = cur->parameter.get();
                continue;
            }

            if (cur->catch_all) {
                std::string rest = seg;
                for (size_t j = i + 1; j < segments.size(); ++j) {
                    rest += '/';
                    rest += segments[j];
                }
                result.params[cur->catch_all_name] = std::move(rest);
                cur = cur->catch_all.get();
                break;
            }

            return std::nullopt;
        }

        if (!cur->is_end) {
            return std::nullopt;
        }

        result.handler = cur->handler.get();

        return result;
    }

private:
    node* fill_parameter(const std::string_view seg, node* cur) {
        const auto name = extract_parameter(seg);

        if (cur->parameter) {
            if (name != cur->parameter_name) {
                throw clover2_http::http::core::routing_error(
                    "Same path for {} and {} parameters.", name,
                    cur->parameter_name);
            }
            return cur->parameter.get();
        }

        cur->parameter = std::make_unique<node>();
        cur->parameter_name = std::string(name);

        return cur->parameter.get();
    }

    node* fill_catch_all(const std::string_view seg, node* cur) {
        const auto name = extract_catch_all(seg);

        if (cur->catch_all) {
            if (name != cur->catch_all_name) {
                throw clover2_http::http::core::routing_error(
                    "Path already exits for {} and {}.", name,
                    cur->catch_all_name);
            }
            return cur->catch_all.get();
        }

        cur->catch_all = std::make_unique<node>();
        cur->catch_all_name = std::string(name);

        return cur->catch_all.get();
    }

    node::UniquePtr root;
};

}  // namespace clover2_http::http::routing
