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
    std::string_view path) noexcept {
    size_t begin = 0;

    while (begin < path.size() && path[begin] == '/') {
        ++begin;
    }

    if (begin == path.size()) {
        return std::nullopt;
    }

    const size_t end = path.find('/', begin);

    return path.substr(begin, end - begin);
}

std::string_view without_first_token(std::string_view path) noexcept {
    const auto begin = path.find_first_not_of('/');

    if (begin == std::string_view::npos) {
        return {};
    }

    const auto end = path.find('/', begin);

    if (end == std::string_view::npos) {
        return {};
    }

    const auto next = path.find_first_not_of('/', end);

    if (next == std::string_view::npos) {
        return {};
    }

    return path.substr(next);
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
