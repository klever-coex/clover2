#include <clover2_http/http/routing/utils.hpp>

namespace clover2_http::http::routing {

std::vector<std::string_view> tokenize_sv(
    const std::string_view path) noexcept {
    std::vector<std::string_view> segs;
    if (path.empty() || path == "/") return segs;

    size_t start = (path[0] == '/') ? 1 : 0;

    while (start < path.size()) {
        size_t end = path.find('/', start);

        if (end == std::string_view::npos) {
            segs.push_back(path.substr(start));
            break;
        }

        if (end > start) segs.push_back(path.substr(start, end - start));
        start = end + 1;
    }

    return segs;
}

std::optional<std::string_view> extract_next_token(
    const std::string_view& path) noexcept {
    while (!path.empty() && path.front() == '/') {
        path.remove_prefix(1);
    }

    if (path.empty()) {
        return std::nullopt;
    }

    size_t end = path.find('/');

    std::string_view token;
    if (end == std::string_view::npos) {
        token = path;
    } else {
        token = path.substr(0, end);
    }

    return token;
}

void remove_first_token(std::string_view& path) noexcept {
    while (!path.empty() && path.front() == '/') {
        path.remove_prefix(1);
    }

    if (path.empty()) {
        return;
    }

    size_t end = path.find('/');

    if (end == std::string_view::npos) {
        path = {};
    } else {
        path.remove_prefix(end + 1);
    }
}

bool is_token_valid(const std::string_view token) noexcept {
    return (is_parameter(token) && token.size() > 2) ||  //
           (is_catch_all(token) && token.size() > 5) ||  //
           (token.front() != '{' && token.back() != '}' && token.size() > 0);
}

bool is_parameter(const std::string_view token) noexcept {
    return token.size() > 2 &&      //
           token.front() == '{' &&  //
           token.back() == '}' &&   //
           !token.ends_with("...}");
}

bool is_catch_all(const std::string_view token) noexcept {
    return token.size() > 5 &&      //
           token.front() == '{' &&  //
           token.ends_with("...}");
}

const std::string_view extract_parameter(
    const std::string_view token) noexcept {
    return token.substr(1, token.size() - 2);
}

const std::string_view extract_catch_all(
    const std::string_view token) noexcept {
    return token.substr(1, token.size() - 5);
}

}  // namespace clover2_http::http::routing
