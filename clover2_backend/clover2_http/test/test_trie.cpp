#include <clover2_http/http/routing/trie.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

using namespace clover2_http::http;
using Handler = std::string;

namespace {

std::unique_ptr<Handler> make_handler(std::string name) {
    return std::make_unique<Handler>(std::move(name));
}

std::vector<std::string> segs(const char* pattern) {
    return routing::split_pattern(pattern);
}

}  // namespace

TEST(Trie, InsertAndSearchStaticRoute) {
    routing::trie<Handler> t;
    t.insert(segs("/api/users"), "/api/users", make_handler("users"));

    auto result = t.search(segs("/api/users"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "users");
    EXPECT_TRUE(result->params.empty());
}

TEST(Trie, StaticRouteNotFound) {
    routing::trie<Handler> t;
    t.insert(segs("/api/users"), "/api/users", make_handler("users"));

    EXPECT_FALSE(t.search(segs("/api/posts")).has_value());
    EXPECT_FALSE(t.search(segs("/api")).has_value());
    EXPECT_FALSE(t.search(segs("/api/users/extra")).has_value());
}

TEST(Trie, MultipleStaticRoutes) {
    routing::trie<Handler> t;
    t.insert(segs("/api/users"), "/api/users", make_handler("users"));
    t.insert(segs("/api/posts"), "/api/posts", make_handler("posts"));
    t.insert(segs("/health"), "/health", make_handler("health"));

    EXPECT_EQ(*t.search(segs("/api/users"))->handler, "users");
    EXPECT_EQ(*t.search(segs("/api/posts"))->handler, "posts");
    EXPECT_EQ(*t.search(segs("/health"))->handler, "health");
}

TEST(Trie, NestedStaticRoutes) {
    routing::trie<Handler> t;
    t.insert(segs("/api"), "/api", make_handler("api-root"));
    t.insert(segs("/api/v1"), "/api/v1", make_handler("api-v1"));
    t.insert(segs("/api/v1/users"), "/api/v1/users",
             make_handler("api-v1-users"));

    EXPECT_EQ(*t.search(segs("/api"))->handler, "api-root");
    EXPECT_EQ(*t.search(segs("/api/v1"))->handler, "api-v1");
    EXPECT_EQ(*t.search(segs("/api/v1/users"))->handler, "api-v1-users");
}

TEST(Trie, RootRoute) {
    routing::trie<Handler> t;
    t.insert(segs("/"), "/", make_handler("root"));

    auto result = t.search(segs("/"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "root");
}

TEST(Trie, SingleParameter) {
    routing::trie<Handler> t;
    t.insert(segs("/api/users/{id}"), "/api/users/{id}",
             make_handler("user-by-id"));

    auto result = t.search(segs("/api/users/42"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "user-by-id");
    EXPECT_EQ(result->params.at("id"), "42");
}

TEST(Trie, MultipleParameters) {
    routing::trie<Handler> t;
    t.insert(segs("/api/{version}/users/{id}"), "/api/{version}/users/{id}",
             make_handler("user-detail"));

    auto result = t.search(segs("/api/v2/users/1337"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "user-detail");
    EXPECT_EQ(result->params.at("version"), "v2");
    EXPECT_EQ(result->params.at("id"), "1337");
}

TEST(Trie, StaticOverParameter) {
    routing::trie<Handler> t;
    t.insert(segs("/api/users/me"), "/api/users/me",
             make_handler("current-user"));
    t.insert(segs("/api/users/{id}"), "/api/users/{id}",
             make_handler("user-by-id"));

    auto static_result = t.search(segs("/api/users/me"));
    ASSERT_TRUE(static_result.has_value());
    EXPECT_EQ(*static_result->handler, "current-user");
    EXPECT_TRUE(static_result->params.empty());

    auto param_result = t.search(segs("/api/users/42"));
    ASSERT_TRUE(param_result.has_value());
    EXPECT_EQ(*param_result->handler, "user-by-id");
    EXPECT_EQ(param_result->params.at("id"), "42");
}

TEST(Trie, SameParameterNameInDifferentBranches) {
    routing::trie<Handler> t;
    t.insert(segs("/users/{id}/profile"), "/users/{id}/profile",
             make_handler("user-profile"));
    t.insert(segs("/posts/{id}/comments"), "/posts/{id}/comments",
             make_handler("post-comments"));

    auto r1 = t.search(segs("/users/10/profile"));
    ASSERT_TRUE(r1.has_value());
    EXPECT_EQ(r1->params.at("id"), "10");

    auto r2 = t.search(segs("/posts/20/comments"));
    ASSERT_TRUE(r2.has_value());
    EXPECT_EQ(r2->params.at("id"), "20");
}

TEST(Trie, CatchAllRoute) {
    routing::trie<Handler> t;
    t.insert(segs("/files/{path...}"), "/files/{path...}",
             make_handler("file-server"));

    auto result = t.search(segs("/files/images/photo.png"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "file-server");
    EXPECT_EQ(result->params.at("path"), "images/photo.png");
}

TEST(Trie, CatchAllAtRoot) {
    routing::trie<Handler> t;
    t.insert(segs("/{catch...}"), "/{catch...}", make_handler("catch-root"));

    auto result = t.search(segs("/anything/at/all"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "catch-root");
    EXPECT_EQ(result->params.at("catch"), "anything/at/all");
}

TEST(Trie, DuplicateRouteReturnsExistingHandler) {
    routing::trie<Handler> t;
    auto* first =
        t.insert(segs("/api/users"), "/api/users", make_handler("first")).first;

    auto [handler, inserted] =
        t.insert(segs("/api/users"), "/api/users", make_handler("second"));
    EXPECT_EQ(handler, first);
    EXPECT_FALSE(inserted);
    EXPECT_EQ(*handler, "first");
}

TEST(Trie, DuplicateParamRouteReturnsExistingHandler) {
    routing::trie<Handler> t;
    auto* first =
        t.insert(segs("/items/{id}"), "/items/{id}", make_handler("by-id"))
            .first;

    auto [handler, inserted] =
        t.insert(segs("/items/{id}"), "/items/{id}", make_handler("again"));
    EXPECT_EQ(handler, first);
    EXPECT_FALSE(inserted);
    EXPECT_EQ(*handler, "by-id");
}

TEST(Trie, CatchAllNotLastThrows) {
    routing::trie<Handler> t;

    EXPECT_THROW(
        {
            t.insert(segs("/{catch...}/extra"), "/{catch...}/extra",
                     make_handler("bad"));
        },
        core::routing_error);
}

TEST(Trie, ConflictingParameterNamesThrows) {
    routing::trie<Handler> t;
    t.insert(segs("/items/{id}"), "/items/{id}", make_handler("by-id"));

    EXPECT_THROW(
        {
            t.insert(segs("/items/{name}"), "/items/{name}",
                     make_handler("by-name"));
        },
        core::routing_error);
}

TEST(Trie, ConflictingCatchAllNamesThrows) {
    routing::trie<Handler> t;
    t.insert(segs("/files/{path...}"), "/files/{path...}", make_handler("fs1"));

    EXPECT_THROW(
        {
            t.insert(segs("/files/{other...}"), "/files/{other...}",
                     make_handler("fs2"));
        },
        core::routing_error);
}

TEST(Trie, SearchEmptyTrie) {
    routing::trie<Handler> t;
    EXPECT_FALSE(t.search(segs("/anything")).has_value());
}

TEST(Trie, SearchOnEmptyPath) {
    routing::trie<Handler> t;
    t.insert(segs("/"), "/", make_handler("root"));

    auto result = t.search(segs("/"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result->handler, "root");
}

TEST(Trie, TrailingSlashIsSeparateBranch) {
    routing::trie<Handler> t;
    t.insert(segs("/nodes"), "/nodes", make_handler("no-slash"));
    t.insert(segs("/nodes/"), "/nodes/", make_handler("with-slash"));

    EXPECT_EQ(*t.search(segs("/nodes"))->handler, "no-slash");
    EXPECT_EQ(*t.search(segs("/nodes/"))->handler, "with-slash");
}

TEST(Trie, StaticOverCatchAll) {
    routing::trie<Handler> t;
    t.insert(segs("/files/list"), "/files/list", make_handler("list"));
    t.insert(segs("/files/{path...}"), "/files/{path...}",
             make_handler("file-server"));

    auto static_result = t.search(segs("/files/list"));
    ASSERT_TRUE(static_result.has_value());
    EXPECT_EQ(*static_result->handler, "list");

    auto catch_all_result = t.search(segs("/files/a/b"));
    ASSERT_TRUE(catch_all_result.has_value());
    EXPECT_EQ(*catch_all_result->handler, "file-server");
    EXPECT_EQ(catch_all_result->params.at("path"), "a/b");
}

TEST(Trie, CatchAllCapturesTrailingEmpty) {
    routing::trie<Handler> t;
    t.insert(segs("/files/{path...}"), "/files/{path...}",
             make_handler("file-server"));

    auto result = t.search(segs("/files/images/"));
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->params.at("path"), "images/");
}

TEST(Trie, ParamRouteExtendedAfterExisting) {
    routing::trie<Handler> t;
    t.insert(segs("/items/{id}"), "/items/{id}", make_handler("by-id"));
    t.insert(segs("/items/{id}/sub"), "/items/{id}/sub",
             make_handler("by-id-sub"));

    EXPECT_EQ(*t.search(segs("/items/42"))->handler, "by-id");
    auto sub = t.search(segs("/items/42/sub"));
    ASSERT_TRUE(sub.has_value());
    EXPECT_EQ(*sub->handler, "by-id-sub");
    EXPECT_EQ(sub->params.at("id"), "42");
}
