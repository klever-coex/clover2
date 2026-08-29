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

TEST(Utils, SplitPatternStripsLeadingSlash) {
    EXPECT_EQ(split_pattern("/nodes"), (std::vector<std::string>{"nodes"}));
    EXPECT_EQ(split_pattern("nodes"), (std::vector<std::string>{"nodes"}));
    EXPECT_EQ(split_pattern("/a/b/c"),
              (std::vector<std::string>{"a", "b", "c"}));
    EXPECT_EQ(split_pattern("/"), (std::vector<std::string>{}));
}

TEST(Utils, SplitPatternPreservesEmptySegments) {
    EXPECT_EQ(split_pattern("/nodes/"), (std::vector<std::string>{"nodes", ""}));
    EXPECT_EQ(split_pattern("/a//b"),
              (std::vector<std::string>{"a", "", "b"}));
    EXPECT_EQ(split_pattern("//"), (std::vector<std::string>{"", ""}));
}
