#include <clover2_http/http/routing/utils.hpp>

using namespace clover2_http::http::routing;

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <gtest/gtest.h>

TEST(Utils, TokenValidation) {
    EXPECT_TRUE(is_token_valid("test"));
    EXPECT_TRUE(is_token_valid("{test}"));
    EXPECT_TRUE(is_token_valid("{test...}"));
    
    EXPECT_FALSE(is_token_valid("{test"));
    EXPECT_FALSE(is_token_valid("test}"));
    EXPECT_FALSE(is_token_valid("{}"));
    EXPECT_FALSE(is_token_valid(""));
}

TEST(Utils, ParameterCheck) {
    EXPECT_TRUE(is_parameter("{test}"));

    EXPECT_FALSE(is_parameter("test"));
    EXPECT_FALSE(is_parameter("{test...}"));
    EXPECT_FALSE(is_parameter("test}"));
    EXPECT_FALSE(is_parameter("{}"));
}

TEST(Utils, CatchAllCheck) {
    EXPECT_TRUE(is_catch_all("{test...}"));

    EXPECT_FALSE(is_catch_all("test"));
    EXPECT_FALSE(is_catch_all("test...}"));
    EXPECT_FALSE(is_catch_all("{test}"));
    EXPECT_FALSE(is_catch_all("{...}"));
}

TEST(Utils, ParameterExtract) {
    EXPECT_EQ(extract_parameter("{test}"), "test");
}

TEST(Utils, CatchAllExtract) {
    EXPECT_EQ(extract_catch_all("{test...}"), "test");
}

TEST(Utils, Tokenize) {
    const std::vector<std::string_view> tokens1 = {"test"};
    const std::vector<std::string_view> tokens2 = {"a", "b", "c"};
    const std::vector<std::string_view> tokens3 = {};
    const std::vector<std::string_view> tokens4 = {"a", "{b}", "c"};

    EXPECT_EQ(tokenize_sv("/test"), tokens1);
    EXPECT_EQ(tokenize_sv("test"), tokens1);
    EXPECT_EQ(tokenize_sv("/a/b/c"), tokens2);
    EXPECT_EQ(tokenize_sv("/a/b/c/"), tokens2);
    EXPECT_EQ(tokenize_sv("/"), tokens3);
    EXPECT_EQ(tokenize_sv(""), tokens3);
    EXPECT_EQ(tokenize_sv("/a/{b}/c"), tokens4);
}

TEST(Utils, ExtractNextTokenSimplePath) {
    std::string_view path = "/api/users";
    
    auto token1 = extract_next_token(path);
    ASSERT_TRUE(token1.has_value());
    EXPECT_EQ(*token1, "api");
    EXPECT_EQ(path, "users");
    
    auto token2 = extract_next_token(path);
    ASSERT_TRUE(token2.has_value());
    EXPECT_EQ(*token2, "users");
    EXPECT_EQ(path, "");
    
    auto token3 = extract_next_token(path);
    EXPECT_FALSE(token3.has_value());
    EXPECT_EQ(path, "");
}

TEST(Utils, ExtractNextTokenOnlySlashes) {
    std::string_view path = "/";
    
    auto token = extract_next_token(path);
    EXPECT_FALSE(token.has_value());
    EXPECT_EQ(path, "");
}

TEST(Utils, ExtractNextTokenParameterToken) {
    std::string_view path = "/api/{id}/posts";
    
    auto token1 = extract_next_token(path);
    ASSERT_TRUE(token1.has_value());
    EXPECT_EQ(*token1, "api");
    EXPECT_EQ(path, "{id}/posts");
    
    auto token2 = extract_next_token(path);
    ASSERT_TRUE(token2.has_value());
    EXPECT_EQ(*token2, "{id}");
    EXPECT_EQ(path, "posts");
    
    EXPECT_TRUE(is_parameter(*token2));
    EXPECT_EQ(extract_parameter(*token2), "id");
}

TEST(Utils, ExtractNextTokenComplexPath) {
    std::string_view path = "/v1/users/{userId}/posts/{postId}/comments";
    
    std::vector<std::string> expected = {"v1", "users", "{userId}", "posts", "{postId}", "comments"};
    std::vector<std::string> actual;
    
    while (auto token = extract_next_token(path)) {
        actual.push_back(std::string(*token));
    }
    
    EXPECT_EQ(actual, expected);
    EXPECT_EQ(path, "");
}
