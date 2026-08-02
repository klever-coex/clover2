#pragma once

#include <clover2_http/http/core/exceptions.hpp>
#include <clover2_http/http/routing/utils.hpp>

#include <format>
#include <memory>
#include <string>
#include <optional>
#include <unordered_map>

namespace clover2_http::http::routing {

template <typename HandlerT>
class trie {
public:
    struct node {
        using UniquePtr = std::unique_ptr<node>;

        node() = default;

        bool is_parameter;
        bool is_end;

        UniquePtr parameter{nullptr};
        std::string parameter_name;

        UniquePtr catch_all{nullptr};
        std::string catch_all_name;

        std::unique_ptr<HandlerT> handler{nullptr};

        std::unordered_map<std::string, node::UniquePtr> children;
    };

    struct match_result {
        HandlerT* handler;
        std::unordered_map<std::string, std::string_view> params;
    };

    trie()
        : root(std::make_unique<node>()) {}

    void insert(const std::string_view route,
                std::unique_ptr<HandlerT> handler) {
        auto cur = root.get();
        const auto tokens = tokenize_sv(route);

        for (const auto& token : tokens) {
            if (!is_token_valid(token)) {
                throw clover2_http::http::core::routing_error(
                    std::format("Invalid path token {}.", token));
            }

            if (is_parameter(token)) {
                cur = fill_parameter(token, cur);

            } else if (is_catch_all(token)) {
                if (&token != &tokens.back()) {
                    throw clover2_http::http::core::routing_error(
                        "Catch all should be the last segment.");
                }

                cur = fill_catch_all(token, cur);
                break;
            } else {
                if (cur->children.find(std::string(token)) == cur->children.end()) {
                    cur->children[std::string(token)] = std::make_unique<node>();
                }

                cur = cur->children[std::string(token)].get();
            }
        }

        if (cur->is_end) {
            throw clover2_http::http::core::routing_error(
                std::format("Route {} already exists", route));
        }

        cur->is_end = true;
        cur->handler = std::move(handler);
    }

    std::optional<match_result> search(const std::string_view route) const {
        auto cur = root.get();
        const auto tokens = tokenize_sv(route);

        match_result result;

        for (const auto& token : tokens) {
            if (!is_token_valid(token)) {
                throw clover2_http::http::core::routing_error(
                    std::format("Invalid path token {}.", token));
            }

            auto it = cur->children.find(std::string(token));
            if (it != cur->children.end()) {
                cur = it->second.get();
                continue;
            }

            if (cur->parameter) {
                result.params[cur->parameter_name] = token;
                cur = cur->parameter.get();
                continue;
            }

            if (cur->catch_all) {
                result.params[cur->catch_all_name] = token;
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
    node* fill_parameter(const std::string_view token, node* cur) {
        const auto name = extract_parameter(token);

        if (cur->parameter && name != cur->parameter_name) {
            throw clover2_http::http::core::routing_error(
                std::format("Same path for {} and {} parameters.", name,
                            cur->parameter_name));
        }

        cur->parameter = std::make_unique<node>();
        cur->parameter_name = std::string(name);

        return cur->parameter.get();
    }

    node* fill_catch_all(const std::string_view token, node* cur) {
        const auto name = extract_catch_all(token);

        if (cur->catch_all && name != cur->catch_all_name) {
            throw clover2_http::http::core::routing_error(
                std::format("Path already exits for {} and {}.", name,
                            cur->catch_all_name));
        }

        cur->catch_all = std::make_unique<node>();
        cur->catch_all_name = std::string(name);

        return cur->catch_all.get();
    }

    node::UniquePtr root;
};

}  // namespace clover2_http::http::routing
