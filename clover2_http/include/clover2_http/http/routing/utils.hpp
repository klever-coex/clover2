#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace clover2_http::http::routing {

std::vector<std::string> split_pattern(const std::string& pattern);

bool is_token_valid(const std::string_view token) noexcept;
bool is_parameter(const std::string_view token) noexcept;
bool is_catch_all(const std::string_view token) noexcept;

const std::string_view extract_parameter(const std::string_view token) noexcept;
const std::string_view extract_catch_all(const std::string_view token) noexcept;

}  // namespace clover2_http::http::routing
