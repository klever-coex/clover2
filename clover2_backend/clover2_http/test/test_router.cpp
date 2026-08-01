#include <clover2_http/endpoint/interface.hpp>
#include <clover2_http/routing/router.hpp>
#include <clover2_http/transport/ws_handler.hpp>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

namespace http = boost::beast::http;
using namespace clover2_http;

endpoint::http_request make_get(std::string target) {
    endpoint::http_request req{http::verb::get, std::move(target),
                               endpoint::kHttpVersion11};
    req.prepare_payload();
    return req;
}

endpoint::http_request make_post(std::string target, std::string body = "") {
    endpoint::http_request req{http::verb::post, std::move(target),
                               endpoint::kHttpVersion11};
    req.body() = std::move(body);
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
    r.dispatch_http(http::verb::get, "/", ctx, make_get("/"), [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(Tokenize, SingleSegment) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/nodes", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "/nodes", ctx, make_get("/nodes"),
                    [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(Tokenize, MultipleSegments) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/a/b/c", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "/a/b/c", ctx, make_get("/a/b/c"),
                    [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(Tokenize, DoubleSlashesIgnored) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/foo/bar", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "//foo//bar", ctx, make_get("//foo//bar"),
                    [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(Tokenize, TrailingSlashIsSeparateRoute) {
    routing::router r;
    auto spy1 = std::make_unique<spy_endpoint>();
    auto spy2 = std::make_unique<spy_endpoint>();
    auto* raw1 = spy1.get();
    auto* raw2 = spy2.get();
    r.add_http_route(http::verb::get, "/nodes", std::move(spy1));
    r.add_http_route(http::verb::get, "/nodes/", std::move(spy2));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "/nodes", ctx, make_get("/nodes"),
                    [](auto) {});
    EXPECT_TRUE(raw1->invoked);
    EXPECT_FALSE(raw2->invoked);
}

TEST(QueryString, StrippedBeforeMatching) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/nodes", std::move(spy));

    core::request_context ctx;
    bool sent = false;
    r.dispatch_http(http::verb::get, "/nodes?key=val&a=b", ctx,
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
    r.dispatch_http(http::verb::get, "/data", ctx, make_get("/data"),
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
    r.dispatch_http(http::verb::post, "/data", ctx, make_post("/data"),
                    [](auto) {});
    EXPECT_TRUE(raw->invoked);
}

TEST(MethodMatch, GetDoesNotMatchPost) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::post, "/data", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "/data", ctx, make_get("/data"),
                    [](auto) {});
    EXPECT_FALSE(raw->invoked);
}

TEST(PathParams, SingleParamExtracted) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    auto* raw = spy.get();
    r.add_http_route(http::verb::get, "/users/{id}", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "/users/42", ctx, make_get("/users/42"),
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
    r.dispatch_http(http::verb::get, "/users/alice/posts/123", ctx,
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
    r.dispatch_http(http::verb::get, "/items/foo-bar", ctx,
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
    r.dispatch_http(http::verb::get, "/a/1", ctx, make_get("/a/1"),
                    [](auto) {});
    ASSERT_TRUE(raw1->invoked);
    EXPECT_EQ(raw1->last_ctx.path_params.at("id"), "1");
    EXPECT_FALSE(raw2->invoked);
}

TEST(NoMatch, ReturnsFalseAndSendsNoResponse) {
    routing::router r;
    core::request_context ctx;
    bool called = false;
    bool found =
        r.dispatch_http(http::verb::get, "/nonexistent", ctx,
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
    r.dispatch_http(http::verb::get, "/a/b", ctx, make_get("/a/b"),
                    [](auto) {});
    EXPECT_FALSE(raw->invoked);
}

TEST(RouteOrder, FirstMatchWins) {
    routing::router r;
    auto spy1 = std::make_unique<spy_endpoint>();
    auto spy2 = std::make_unique<spy_endpoint>();
    auto* raw1 = spy1.get();
    auto* raw2 = spy2.get();
    // Identical patterns — first registered wins
    r.add_http_route(http::verb::get, "/dup", std::move(spy1));
    r.add_http_route(http::verb::get, "/dup", std::move(spy2));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "/dup", ctx, make_get("/dup"),
                    [](auto) {});
    EXPECT_TRUE(raw1->invoked);
    EXPECT_FALSE(raw2->invoked);
}

TEST(WebSocket, StaticRouteMatches) {
    routing::router r;
    auto handler = std::make_unique<spy_ws_handler>();
    auto* raw = handler.get();
    r.add_ws_route("/ws/chat", std::move(handler));

    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws("/ws/chat", params);
    EXPECT_EQ(found, raw);
    EXPECT_TRUE(params.empty());
}

TEST(WebSocket, ParamRouteExtractsParams) {
    routing::router r;
    auto handler = std::make_unique<spy_ws_handler>();
    r.add_ws_route("/ws/{room}", std::move(handler));

    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws("/ws/lobby", params);
    EXPECT_NE(found, nullptr);
    EXPECT_EQ(params.at("room"), "lobby");
}

TEST(WebSocket, NoMatchReturnsNull) {
    routing::router r;
    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws("/no/such/path", params);
    EXPECT_EQ(found, nullptr);
}

TEST(WebSocket, QueryStrippedBeforeMatch) {
    routing::router r;
    auto handler = std::make_unique<spy_ws_handler>();
    auto* raw = handler.get();
    r.add_ws_route("/ws/chat", std::move(handler));

    std::unordered_map<std::string, std::string> params;
    auto* found = r.match_ws("/ws/chat?token=abc", params);
    EXPECT_EQ(found, raw);
}

TEST(Logger, NullLoggerDoesNotCrash) {
    routing::router r;
    auto spy = std::make_unique<spy_endpoint>();
    r.add_http_route(http::verb::get, "/test", std::move(spy));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "/test", ctx, make_get("/test"),
                    [](auto) {});
    r.dispatch_http(http::verb::get, "/nonexistent", ctx,
                    make_get("/nonexistent"), [](auto) {});

    std::unordered_map<std::string, std::string> params;
    r.match_ws("/test", params);
}

TEST(ResponseSender, EndpointInvokedWithResponse) {
    routing::router r;
    auto ep = std::make_unique<spy_endpoint>();
    r.add_http_route(http::verb::get, "/ok", std::move(ep));

    core::request_context ctx;
    http::response<http::string_body> captured{http::status::unknown,
                                               endpoint::kHttpVersion11};
    r.dispatch_http(http::verb::get, "/ok", ctx, make_get("/ok"),
                    [&](auto resp) { captured = std::move(resp); });
    EXPECT_EQ(captured.result(), http::status::ok);
}

TEST(SegmentGuard, RoutesWithWrongSegmentCountSkipped) {
    routing::router r;
    auto spy3 = std::make_unique<spy_endpoint>();
    auto* raw3 = spy3.get();
    r.add_http_route(http::verb::get, "/a/b/c", std::move(spy3));

    core::request_context ctx;
    r.dispatch_http(http::verb::get, "/a/b", ctx, make_get("/a/b"),
                    [](auto) {});
    EXPECT_FALSE(raw3->invoked);

    r.dispatch_http(http::verb::get, "/a/b/c/d", ctx, make_get("/a/b/c/d"),
                    [](auto) {});
    EXPECT_FALSE(raw3->invoked);

    r.dispatch_http(http::verb::get, "/a/b/c", ctx, make_get("/a/b/c"),
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
    r.add_http_route(http::verb::get, "/users/{user_id}/posts/{post_id}",
                     std::move(ep_post));
    r.add_http_route(http::verb::post, "/static", std::move(ep_static));
    r.add_http_route(http::verb::get, "/wild/{rest}", std::move(ep_wild));

    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, "/nodes", ctx, make_get("/nodes"),
                        [](auto) {});
        EXPECT_TRUE(raw_nodes->invoked);
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, "/users/42", ctx,
                        make_get("/users/42"), [](auto) {});
        EXPECT_TRUE(raw_user->invoked);
        EXPECT_EQ(raw_user->last_ctx.path_params.at("id"), "42");
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, "/users/alice/posts/789", ctx,
                        make_get("/users/alice/posts/789"), [](auto) {});
        EXPECT_TRUE(raw_post->invoked);
        EXPECT_EQ(raw_post->last_ctx.path_params.at("user_id"), "alice");
        EXPECT_EQ(raw_post->last_ctx.path_params.at("post_id"), "789");
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::post, "/static", ctx, make_post("/static"),
                        [](auto) {});
        EXPECT_TRUE(raw_static->invoked);
    }
    {
        core::request_context ctx;
        r.dispatch_http(http::verb::get, "/wild/something", ctx,
                        make_get("/wild/something"), [](auto) {});
        EXPECT_TRUE(raw_wild->invoked);
        EXPECT_EQ(raw_wild->last_ctx.path_params.at("rest"), "something");
    }
}

}  // namespace
