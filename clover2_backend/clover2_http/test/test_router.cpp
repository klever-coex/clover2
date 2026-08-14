#include <clover2_http/http/endpoint/interface.hpp>
#include <clover2_http/http/routing/router.hpp>
#include <clover2_http/http/transport/ws_handler.hpp>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/url.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

namespace http = boost::beast::http;
using namespace clover2_http::http;

// Parses a target the same way http_session does. Only string literals:
// the returned url_view references the literal's static storage.
boost::urls::url_view make_target(const char* t) {
    auto r = boost::urls::parse_relative_ref(t);
    EXPECT_TRUE(!r.has_error());
    return *r;
}

endpoint::http_request make_get(std::string target) {
    endpoint::http_request req{http::verb::get, std::move(target),
                               11};
    req.prepare_payload();
    return req;
}

endpoint::http_request make_post(std::string target, std::string body = "") {
    endpoint::http_request req{http::verb::post, std::move(target),
                               11};
    req.body() = std::move(body);
    req.prepare_payload();
    return req;
}

endpoint::http_request make_request(http::verb method, std::string target) {
    endpoint::http_request req{method, std::move(target), 11};
    req.prepare_payload();
    return req;
}

struct spy_endpoint : public endpoint::interface {
    bool invoked = false;
    core::request_context last_ctx;
    int last_status = 0;

    void invoke(core::request_context ctx, endpoint::http_request /*req*/,
                endpoint::response_sender sender) override {
        invoked = true;
        last_ctx = std::move(ctx);
        sender(endpoint::make_ok_response("{}"));
    }
};

struct status_endpoint : public endpoint::interface {
    int status;
    explicit status_endpoint(int s)
        : status(s) {}
    void invoke(core::request_context, endpoint::http_request,
                endpoint::response_sender sender) override {
        sender(endpoint::make_error_response(status, "test"));
    }
};

struct spy_ws_handler : public transport::ws_handler_interface {
    bool accepted = false;
    core::request_context last_ctx;

    void on_accept(boost::asio::ip::tcp::socket,
                   http::request<http::string_body>,
                   core::request_context ctx) override {
        accepted = true;
        last_ctx = std::move(ctx);
    }
};

TEST(Tokenize, EmptyPathReturnsNoSegments) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/"), ctx, make_get("/"), [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(Tokenize, SingleSegment) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/nodes", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/nodes"), ctx, make_get("/nodes"),
                    [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(Tokenize, MultipleSegments) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/a/b/c", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/a/b/c"), ctx, make_get("/a/b/c"),
                    [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(Tokenize, NetworkPathReferenceDoesNotMatch) {
    // RFC 3986: "//foo//bar" is a network-path reference (authority "foo",
    // path "//bar"), so it does not match the "/foo/bar" route.
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/foo/bar", std::move(spy));

    core::request_context ctx;
    bool found = r.dispatch_http(http::verb::get, make_target("//foo//bar"),
                                 ctx, make_get("//foo//bar"), [](auto) {});
    EXPECT_FALSE(found);
    EXPECT_FALSE(raw->invoked);
}

TEST(Tokenize, TrailingSlashIsSeparateRoute) {
    routing::router r;
    auto spy1 = std::make_unique<spy_endpoint>();
    auto spy2 = std::make_unique<spy_endpoint>();
    auto* raw1 = spy1.get();
    auto* raw2 = spy2.get();
    r.add_http_route(http::verb::get, "/nodes", std::move(spy1));
    r.add_http_route(http::verb::get, "/nodes/", std::move(spy2));

    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/nodes"), ctx,
                        make_get("/nodes"), [](auto) {});
        EXPECT_TRUE(raw1->invoked);
        EXPECT_FALSE(raw2->invoked);
    }
    raw1->invoked = false;
    raw2->invoked = false;
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/nodes/"), ctx,
                        make_get("/nodes/"), [](auto) {});
        EXPECT_FALSE(raw1->invoked);
        EXPECT_TRUE(raw2->invoked);
    }
}

TEST(QueryString, StrippedBeforeMatching) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/nodes", std::move(spy));

    core::request_context ctx;
    bool sent = false;
    r.dispatch_http(http::verb::get, make_target("/nodes?key=val&a=b"), ctx,
                    make_get("/nodes?key=val&a=b"), [&](auto) { sent = true; });
    EXPECT_TRUE(raw->invoked);
    EXPECT_TRUE(sent);
    EXPECT_TRUE(ctx.query_params.empty());
}

TEST(MethodMatch, CorrectMethodMatches) {
    routing::router r;
    auto spy_get = std::make_unique<spy_endpoint>();
    auto spy_post = std::make_unique<spy_endpoint>();
    auto* raw_get = spy_get.get();
    auto* raw_post = spy_post.get();
    r.add_http_route(http::verb::get, "/data", std::move(spy_get));
    r.add_http_route(http::verb::post, "/data", std::move(spy_post));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/data"), ctx, make_get("/data"),
                    [](auto) {});
    EXPECT_TRUE(raw_get->invoked);
    EXPECT_FALSE(raw_post->invoked);
}

TEST(MethodMatch, PostMatchesPost) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::post, "/data", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::post, make_target("/data"), ctx, make_post("/data"),
                    [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(MethodMatch, GetDoesNotMatchPost) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::post, "/data", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/data"), ctx, make_get("/data"),
                    [](auto) {});
    EXPECT_FALSE(raw->invoked);
}

TEST(PathParams, SingleParamExtracted) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/users/{id}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/users/42"), ctx, make_get("/users/42"),
                    [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.path_params.at("id"), "42");
}

TEST(PathParams, MultipleParamsExtracted) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/users/{user_id}/posts/{post_id}",
                     std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/users/alice/posts/123"), ctx,
                    make_get("/users/alice/posts/123"), [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.path_params.at("user_id"), "alice");
    EXPECT_EQ(raw->last_ctx.path_params.at("post_id"), "123");
}

TEST(PathParams, ParamWithDash) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/items/{item-id}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/items/foo-bar"), ctx,
                    make_get("/items/foo-bar"), [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.path_params.at("item-id"), "foo-bar");
}

TEST(PathParams, TwoRoutesSameParamNameDifferentPaths) {
    routing::router r;
    auto spy1 = std::make_unique<spy_endpoint>();
    auto spy2 = std::make_unique<spy_endpoint>();
    auto* raw1 = spy1.get();
    auto* raw2 = spy2.get();
    r.add_http_route(http::verb::get, "/a/{id}", std::move(spy1));
    r.add_http_route(http::verb::get, "/b/{id}", std::move(spy2));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/a/1"), ctx, make_get("/a/1"),
                    [](auto) {});
    ASSERT_TRUE(raw1->invoked);
    EXPECT_EQ(raw1->last_ctx.path_params.at("id"), "1");
    EXPECT_FALSE(raw2->invoked);
}

TEST(PathParams, TypedHelper) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/users/{id}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/users/42"), ctx,
                    make_get("/users/42"), [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.param<int>("id"), 42);
    EXPECT_EQ(raw->last_ctx.param<std::string>("id"), "42");
}

TEST(PathParams, PercentDecoded) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/users/{id}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/users/John%20Doe"), ctx,
                    make_get("/users/John%20Doe"), [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.path_params.at("id"), "John Doe");
}

TEST(QueryParams, PercentDecoded) {
    core::request_context ctx(make_target("/x?name=John%20Doe"));
    ASSERT_EQ(ctx.query_params.size(), 1u);
    EXPECT_EQ(ctx.query_params[0].key, "name");
    EXPECT_EQ(ctx.query_params[0].value, "John Doe");
    EXPECT_TRUE(ctx.query_params[0].has_value);
}

TEST(QueryParams, DuplicatesPreserved) {
    core::request_context ctx(make_target("/x?tag=a&tag=b"));
    ASSERT_EQ(ctx.query_params.size(), 2u);
    EXPECT_EQ(ctx.query_params[0].key, "tag");
    EXPECT_EQ(ctx.query_params[0].value, "a");
    EXPECT_EQ(ctx.query_params[1].key, "tag");
    EXPECT_EQ(ctx.query_params[1].value, "b");
}

TEST(QueryParams, EmptyValueVsNoValue) {
    core::request_context ctx(make_target("/x?empty&empty2="));
    ASSERT_EQ(ctx.query_params.size(), 2u);
    EXPECT_EQ(ctx.query_params[0].key, "empty");
    EXPECT_FALSE(ctx.query_params[0].has_value);
    EXPECT_EQ(ctx.query_params[1].key, "empty2");
    EXPECT_TRUE(ctx.query_params[1].has_value);
    EXPECT_EQ(ctx.query_params[1].value, "");
}

TEST(QueryParams, PlusDecodedAsSpace) {
    // Boost.URL decodes the query as form-urlencoded: '+' becomes a space.
    core::request_context ctx(make_target("/x?q=a+b"));
    ASSERT_EQ(ctx.query_params.size(), 1u);
    EXPECT_EQ(ctx.query_params[0].value, "a b");
}

TEST(QueryParams, TypedHelpers) {
    core::request_context ctx(make_target("/x?n=42&s=hello&flag=true&n=7"));
    EXPECT_EQ(ctx.query<int>("n"), 7);  // last occurrence wins
    EXPECT_EQ(ctx.query<std::string>("s"), "hello");
    EXPECT_EQ(ctx.query<bool>("flag"), true);
    EXPECT_EQ(ctx.query<int>("missing"), 0);
    EXPECT_EQ(ctx.query<std::string>("missing", "dflt"), "dflt");
}

TEST(QueryParams, UrlMemberHoldsFullTarget) {
    core::request_context ctx(make_target("/nodes?tag=a&tag=b"));
    EXPECT_EQ(ctx.url.path(), "/nodes");
    auto params = ctx.url.params();
    ASSERT_EQ(params.size(), 2u);
    auto it = params.begin();
    EXPECT_EQ((*it).value, "a");
    ++it;
    EXPECT_EQ((*it).value, "b");
}

TEST(NoMatch, ReturnsFalseAndSendsNoResponse) {
    routing::router r;
    core::request_context ctx;
    bool called = false;
    bool found =
        r.dispatch_http(http::verb::get, make_target("/nonexistent"), ctx,
                        make_get("/nonexistent"), [&](auto) { called = true; });
    EXPECT_FALSE(found);
    EXPECT_FALSE(called);
}

TEST(NoMatch, RouteWithDifferentSegmentCount) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/a/b/c", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/a/b"), ctx, make_get("/a/b"),
                    [](auto) {});
    EXPECT_FALSE(raw->invoked);
}

TEST(RouteOrder, DuplicatePatternThrows) {
    routing::router r;
    r.add_http_route(http::verb::get, "/dup", std::make_unique<spy_endpoint>());

    EXPECT_THROW(
        { r.add_http_route(http::verb::get, "/dup",
                           std::make_unique<spy_endpoint>()); },
        core::routing_error);
}

TEST(WebSocket, StaticRouteMatches) {
    routing::router r;
    auto handler = std::make_unique<spy_ws_handler>();
    auto* raw = handler.get();
    r.add_ws_route("/ws/chat", std::move(handler));

    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws(make_target("/ws/chat"), params);
    EXPECT_EQ(found, raw);
    EXPECT_TRUE(params.empty());
}

TEST(WebSocket, ParamRouteExtractsParams) {
    routing::router r;
    auto handler = std::make_unique<spy_ws_handler>();
    r.add_ws_route("/ws/{room}", std::move(handler));

    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws(make_target("/ws/lobby"), params);
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(params.at("room"), "lobby");
}

TEST(WebSocket, NoMatchReturnsNull) {
    routing::router r;
    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws(make_target("/no/such/path"), params);
    EXPECT_EQ(found, nullptr);
}

TEST(WebSocket, QueryStrippedBeforeMatch) {
    routing::router r;
    auto handler = std::make_unique<spy_ws_handler>();
    auto* raw = handler.get();
    r.add_ws_route("/ws/chat", std::move(handler));

    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws(make_target("/ws/chat?token=abc"), params);
    EXPECT_EQ(found, raw);
}

TEST(Logger, NullLoggerDoesNotCrash) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    r.add_http_route(http::verb::get, "/test", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/test"), ctx, make_get("/test"),
                    [](auto) {});
    r.dispatch_http(http::verb::get, make_target("/nonexistent"), ctx,
                    make_get("/nonexistent"), [](auto) {});

    std::unordered_map<std::string, std::string> params;
    r.match_ws(make_target("/test"), params);
}

TEST(ResponseSender, EndpointInvokedWithResponse) {
    routing::router r;
    auto ep = std::make_unique<spy_endpoint>();
    r.add_http_route(http::verb::get, "/ok", std::move(ep));

    core::request_context ctx;
    http::response<http::string_body> captured{http::status::unknown,
                                               11};
    r.dispatch_http(http::verb::get, make_target("/ok"), ctx, make_get("/ok"),
                    [&](auto resp) { captured = std::move(resp); });
    EXPECT_EQ(captured.result(), http::status::ok);
}

TEST(SegmentGuard, RoutesWithWrongSegmentCountSkipped) {
    routing::router r;
    auto spy3 = std::make_unique<spy_endpoint>();
    auto* raw3 = spy3.get();
    r.add_http_route(http::verb::get, "/a/b/c", std::move(spy3));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/a/b"), ctx, make_get("/a/b"),
                    [](auto) {});
    EXPECT_FALSE(raw3->invoked);

    r.dispatch_http(http::verb::get, make_target("/a/b/c/d"), ctx, make_get("/a/b/c/d"),
                    [](auto) {});
    EXPECT_FALSE(raw3->invoked);

    r.dispatch_http(http::verb::get, make_target("/a/b/c"), ctx, make_get("/a/b/c"),
                    [](auto) {});
    EXPECT_TRUE(raw3->invoked);
}

TEST(ComplexTable, MultipleRoutesWithParams) {
    routing::router r;
    auto ep_nodes = std::make_unique<spy_endpoint>();
    auto ep_user = std::make_unique<spy_endpoint>();
    auto ep_post = std::make_unique<spy_endpoint>();
    auto ep_static = std::make_unique<spy_endpoint>();
    auto ep_wild = std::make_unique<spy_endpoint>();
    auto* raw_nodes = ep_nodes.get();
    auto* raw_user = ep_user.get();
    auto* raw_post = ep_post.get();
    auto* raw_static = ep_static.get();
    auto* raw_wild = ep_wild.get();

    r.add_http_route(http::verb::get, "/nodes", std::move(ep_nodes));
    r.add_http_route(http::verb::get, "/users/{id}", std::move(ep_user));
    // Same parameter name at the same position as /users/{id}: the trie
    // requires all routes sharing a position to use one parameter name.
    r.add_http_route(http::verb::get, "/users/{id}/posts/{post_id}",
                     std::move(ep_post));
    r.add_http_route(http::verb::post, "/static", std::move(ep_static));
    r.add_http_route(http::verb::get, "/wild/{rest}", std::move(ep_wild));

    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/nodes"), ctx, make_get("/nodes"),
                        [](auto) {});
        EXPECT_TRUE(raw_nodes->invoked);
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/users/42"), ctx,
                        make_get("/users/42"), [](auto) {});
        EXPECT_TRUE(raw_user->invoked);
        EXPECT_EQ(raw_user->last_ctx.path_params.at("id"), "42");
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/users/alice/posts/789"), ctx,
                        make_get("/users/alice/posts/789"), [](auto) {});
        EXPECT_TRUE(raw_post->invoked);
        EXPECT_EQ(raw_post->last_ctx.path_params.at("id"), "alice");
        EXPECT_EQ(raw_post->last_ctx.path_params.at("post_id"), "789");
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::post, make_target("/static"), ctx, make_post("/static"),
                        [](auto) {});
        EXPECT_TRUE(raw_static->invoked);
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/wild/something"), ctx,
                        make_get("/wild/something"), [](auto) {});
        EXPECT_TRUE(raw_wild->invoked);
        EXPECT_EQ(raw_wild->last_ctx.path_params.at("rest"), "something");
    }
}

TEST(CatchAll, CapturesRemainingSegments) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/files/{path...}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/files/images/photo.png"), ctx,
                    make_get("/files/images/photo.png"), [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.path_params.at("path"), "images/photo.png");
}

TEST(CatchAll, AtRoot) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/{rest...}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/a/b/c"), ctx, make_get("/a/b/c"),
                    [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.path_params.at("rest"), "a/b/c");
}

TEST(CatchAll, StaticWins) {
    routing::router r;
    auto spy_static = std::make_unique<spy_endpoint>();
    auto spy_catch = std::make_unique<spy_endpoint>();
    auto* raw_static = spy_static.get();
    auto* raw_catch = spy_catch.get();
    r.add_http_route(http::verb::get, "/files/{path...}", std::move(spy_catch));
    r.add_http_route(http::verb::get, "/files/list", std::move(spy_static));

    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/files/list"), ctx,
                        make_get("/files/list"), [](auto) {});
        EXPECT_TRUE(raw_static->invoked);
        EXPECT_FALSE(raw_catch->invoked);
    }
    raw_static->invoked = false;
    raw_catch->invoked = false;
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/files/a/b"), ctx,
                        make_get("/files/a/b"), [](auto) {});
        EXPECT_FALSE(raw_static->invoked);
        EXPECT_TRUE(raw_catch->invoked);
        EXPECT_EQ(raw_catch->last_ctx.path_params.at("path"), "a/b");
    }
}

TEST(CatchAll, MatchesTrailingSlash) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/files/{path...}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/files/"), ctx, make_get("/files/"),
                    [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.path_params.at("path"), "");
}

TEST(CatchAll, NotLastThrows) {
    routing::router r;
    EXPECT_THROW(
        { r.add_http_route(http::verb::get, "/{rest...}/extra",
                           std::make_unique<spy_endpoint>()); },
        core::routing_error);
}

TEST(CatchAll, NameConflictThrows) {
    routing::router r;
    r.add_http_route(http::verb::get, "/files/{path...}",
                     std::make_unique<spy_endpoint>());

    EXPECT_THROW(
        { r.add_http_route(http::verb::get, "/files/{other...}",
                           std::make_unique<spy_endpoint>()); },
        core::routing_error);
}

TEST(Conflicts, ParamNameConflictThrows) {
    routing::router r;
    r.add_http_route(http::verb::get, "/items/{id}", std::make_unique<spy_endpoint>());

    EXPECT_THROW(
        { r.add_http_route(http::verb::get, "/items/{name}",
                           std::make_unique<spy_endpoint>()); },
        core::routing_error);
}

TEST(Conflicts, InvalidPatternThrows) {
    routing::router r;
    EXPECT_THROW(
        { r.add_http_route(http::verb::get, "/x/{bad",
                           std::make_unique<spy_endpoint>()); },
        core::routing_error);
}

TEST(Precedence, StaticWinsRegardlessOfRegistrationOrder) {
    routing::router r;
    auto spy_param = std::make_unique<spy_endpoint>();
    auto spy_static = std::make_unique<spy_endpoint>();
    auto* raw_param = spy_param.get();
    auto* raw_static = spy_static.get();
    // Parameter route registered first — static still wins on exact match.
    r.add_http_route(http::verb::get, "/users/{id}", std::move(spy_param));
    r.add_http_route(http::verb::get, "/users/me", std::move(spy_static));

    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/users/me"), ctx,
                        make_get("/users/me"), [](auto) {});
        EXPECT_TRUE(raw_static->invoked);
        EXPECT_FALSE(raw_param->invoked);
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/users/42"), ctx,
                        make_get("/users/42"), [](auto) {});
        EXPECT_TRUE(raw_param->invoked);
        EXPECT_EQ(raw_param->last_ctx.path_params.at("id"), "42");
    }
}

TEST(PathParams, EncodedSlashStaysInOneParam) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/users/{id}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, make_target("/users/a%2Fb"), ctx,
                    make_get("/users/a%2Fb"), [](auto) {});
    ASSERT_TRUE(raw->invoked);
    EXPECT_EQ(raw->last_ctx.path_params.at("id"), "a/b");
}

TEST(MethodMatch, ThreeVerbsCoexistOnOnePattern) {
    routing::router r;
    auto spy_get = std::make_unique<spy_endpoint>();
    auto spy_put = std::make_unique<spy_endpoint>();
    auto spy_patch = std::make_unique<spy_endpoint>();
    auto* raw_get = spy_get.get();
    auto* raw_put = spy_put.get();
    auto* raw_patch = spy_patch.get();
    r.add_http_route(http::verb::get, "/multi", std::move(spy_get));
    r.add_http_route(http::verb::put, "/multi", std::move(spy_put));
    r.add_http_route(http::verb::patch, "/multi", std::move(spy_patch));

    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, make_target("/multi"), ctx,
                        make_request(http::verb::get, "/multi"), [](auto) {});
        EXPECT_TRUE(raw_get->invoked);
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::put, make_target("/multi"), ctx,
                        make_request(http::verb::put, "/multi"), [](auto) {});
        EXPECT_TRUE(raw_put->invoked);
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::patch, make_target("/multi"), ctx,
                        make_request(http::verb::patch, "/multi"), [](auto) {});
        EXPECT_TRUE(raw_patch->invoked);
    }
}

TEST(WebSocket, DuplicateRouteThrows) {
    routing::router r;
    r.add_ws_route("/ws/dup", std::make_unique<spy_ws_handler>());

    EXPECT_THROW(
        { r.add_ws_route("/ws/dup", std::make_unique<spy_ws_handler>()); },
        core::routing_error);
}

TEST(WebSocket, CatchAllCapturesRemainingSegments) {
    routing::router r;
    auto handler = std::make_unique<spy_ws_handler>();
    r.add_ws_route("/ws/{room...}", std::move(handler));

    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws(make_target("/ws/a/b"), params);
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(params.at("room"), "a/b");
}

}  // namespace
