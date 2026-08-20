#include <clover2_http/http/routing/utils.hpp>

namespace clover2_http::http::routing {

std::vector<std::string> split_pattern(const std::string& pattern) {
    std::vector<std::string> segs;
    size_t start = (!pattern.empty() && pattern[0] == '/') ? 1 : 0;

    while (start < pattern.size()) {
        size_t end = pattern.find('/', start);
        if (end == std::string::npos) {
            segs.emplace_back(pattern.substr(start));
            break;
        }
        segs.emplace_back(pattern.substr(start, end - start));
        start = end + 1;
    }

    if (pattern.size() > 1 && pattern.back() == '/') {
        segs.emplace_back("");
    }

    return segs;
}

bool is_token_valid(const std::string_view token) noexcept {
    return is_parameter(token) ||                                          //
           is_catch_all(token) ||                                          //
           (!token.empty() && token.front() != '{' && token.back() != '}');
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
