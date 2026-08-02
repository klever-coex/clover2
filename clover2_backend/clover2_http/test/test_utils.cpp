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
