#pragma once

#include <string>
#include <vector>

namespace clover2_http::http::routing {

std::vector<std::string_view> tokenize_sv(const std::string_view path) noexcept;

bool is_token_valid(const std::string_view token) noexcept;
bool is_parameter(const std::string_view token) noexcept;
bool is_catch_all(const std::string_view token) noexcept;

const std::string_view extract_parameter(const std::string_view token) noexcept;
const std::string_view extract_catch_all(const std::string_view token) noexcept;

}  // namespace clover2_http::http::routing
