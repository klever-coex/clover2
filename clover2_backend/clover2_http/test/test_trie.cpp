#include <clover2_http/http/routing/trie.hpp>

using namespace clover2_http::http;

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using Handler = std::string;

namespace {

std::unique_ptr<Handler> make_handler(std::string name) {
    return std::make_unique<Handler>(std::move(name));
}

}  // namespace

TEST(Trie, InsertAndSearchStaticRoute) {
    routing::trie<Handler> t;
    t.insert("/api/users", make_handler("users"));

    auto result = t.search("/api/users");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "users");
    EXPECT_TRUE(result->params.empty());
}

TEST(Trie, StaticRouteNotFound) {
    routing::trie<Handler> t;
    t.insert("/api/users", make_handler("users"));

    EXPECT_FALSE(t.search("/api/posts").has_value());
    EXPECT_FALSE(t.search("/api").has_value());
    EXPECT_FALSE(t.search("/api/users/extra").has_value());
}

TEST(Trie, MultipleStaticRoutes) {
    routing::trie<Handler> t;
    t.insert("/api/users", make_handler("users"));
    t.insert("/api/posts", make_handler("posts"));
    t.insert("/health", make_handler("health"));

    EXPECT_EQ(*t.search("/api/users")->handler, "users");
    EXPECT_EQ(*t.search("/api/posts")->handler, "posts");
    EXPECT_EQ(*t.search("/health")->handler, "health");
}

TEST(Trie, NestedStaticRoutes) {
    routing::trie<Handler> t;
    t.insert("/api", make_handler("api-root"));
    t.insert("/api/v1", make_handler("api-v1"));
    t.insert("/api/v1/users", make_handler("api-v1-users"));

    EXPECT_EQ(*t.search("/api")->handler, "api-root");
    EXPECT_EQ(*t.search("/api/v1")->handler, "api-v1");
    EXPECT_EQ(*t.search("/api/v1/users")->handler, "api-v1-users");
}

TEST(Trie, RootRoute) {
    routing::trie<Handler> t;
    t.insert("/", make_handler("root"));

    auto result = t.search("/");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "root");
}

TEST(Trie, SingleParameter) {
    routing::trie<Handler> t;
    t.insert("/api/users/{id}", make_handler("user-by-id"));

    auto result = t.search("/api/users/42");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "user-by-id");
    EXPECT_EQ(result->params.at("id"), "42");
}

TEST(Trie, MultipleParameters) {
    routing::trie<Handler> t;
    t.insert("/api/{version}/users/{id}", make_handler("user-detail"));

    auto result = t.search("/api/v2/users/1337");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "user-detail");
    EXPECT_EQ(result->params.at("version"), "v2");
    EXPECT_EQ(result->params.at("id"), "1337");
}

TEST(Trie, StaticOverParameter) {
    routing::trie<Handler> t;
    t.insert("/api/users/me", make_handler("current-user"));
    t.insert("/api/users/{id}", make_handler("user-by-id"));

    auto static_result = t.search("/api/users/me");
    ASSERT_TRUE(static_result.has_value());
    EXPECT_EQ(*static_result->handler, "current-user");
    EXPECT_TRUE(static_result->params.empty());

    auto param_result = t.search("/api/users/42");
    ASSERT_TRUE(param_result.has_value());
    EXPECT_EQ(*param_result->handler, "user-by-id");
    EXPECT_EQ(param_result->params.at("id"), "42");
}

TEST(Trie, SameParameterNameInDifferentBranches) {
    routing::trie<Handler> t;
    t.insert("/users/{id}/profile", make_handler("user-profile"));
    t.insert("/posts/{id}/comments", make_handler("post-comments"));

    auto r1 = t.search("/users/10/profile");
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(r1->params.at("id"), "10");

    auto r2 = t.search("/posts/20/comments");
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2->params.at("id"), "20");
}

TEST(Trie, CatchAllRoute) {
    routing::trie<Handler> t;
    t.insert("/files/{path...}", make_handler("file-server"));

    auto result = t.search("/files/images/photo.png");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "file-server");
    EXPECT_EQ(result->params.at("path"), "images/photo.png");
}

TEST(Trie, CatchAllAtRoot) {
    routing::trie<Handler> t;
    t.insert("/{catch...}", make_handler("catch-root"));

    auto result = t.search("/anything/at/all");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "catch-root");
}

TEST(Trie, DuplicateRouteThrows) {
    routing::trie<Handler> t;
    t.insert("/api/users", make_handler("first"));

    EXPECT_THROW(
        { t.insert("/api/users", make_handler("second")); },
        core::routing_error);
}

TEST(Trie, CatchAllNotLastThrows) {
    routing::trie<Handler> t;

    EXPECT_THROW(
        { t.insert("/{catch...}/extra", make_handler("bad")); },
        core::routing_error);
}

TEST(Trie, ConflictingParameterNamesThrows) {
    routing::trie<Handler> t;
    t.insert("/items/{id}", make_handler("by-id"));

    EXPECT_THROW(
        { t.insert("/items/{name}", make_handler("by-name")); },
        core::routing_error);
}

TEST(Trie, ConflictingCatchAllNamesThrows) {
    routing::trie<Handler> t;
    t.insert("/files/{path...}", make_handler("fs1"));

    EXPECT_THROW(
        { t.insert("/files/{other...}", make_handler("fs2")); },
        core::routing_error);
}

TEST(Trie, SearchEmptyTrie) {
    routing::trie<Handler> t;
    EXPECT_FALSE(t.search("/anything").has_value());
}

TEST(Trie, SearchOnEmptyPath) {
    routing::trie<Handler> t;
    t.insert("/", make_handler("root"));

    auto result = t.search("/");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "root");
}
