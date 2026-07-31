// ./a.out
// curl http://0.0.0.0:8080/hello
// curl http://0.0.0.0:8080/users/42
// curl -X POST http://0.0.0.0:8080/echo -d '{"text":"hi"}'
// websocat ws://0.0.0.0:8080/ws/chat

#include <clover2_http/core/logger.hpp>
#include <clover2_http/serialization/json_traits.hpp>
#include <clover2_http/server.hpp>

#include <boost/asio/signal_set.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

struct EchoRequest {
    std::string text;
};

struct EchoResponse {
    std::string echoed;
    int length;
};

struct UserResponse {
    int id;
    std::string name;
};

struct ChatMessage {
    std::string user;
    std::string body;
};

template <>
struct clover2_http::serialization::json_traits<EchoRequest> {
    static EchoRequest from_json(const nlohmann::json& jv) {
        return {.text = jv.at("text").get<std::string>()};
    }
    static void to_json(nlohmann::json& jv, const EchoRequest& obj) {
        jv["text"] = obj.text;
    }
};

template <>
struct clover2_http::serialization::json_traits<EchoResponse> {
    static EchoResponse from_json(const nlohmann::json& jv) {
        return {.echoed = jv.at("echoed").get<std::string>(),
                .length = jv.at("length").get<int>()};
    }
    static void to_json(nlohmann::json& jv, const EchoResponse& obj) {
        jv["echoed"] = obj.echoed;
        jv["length"] = obj.length;
    }
};

template <>
struct clover2_http::serialization::json_traits<UserResponse> {
    static UserResponse from_json(const nlohmann::json& jv) {
        return {.id = jv.at("id").get<int>(),
                .name = jv.at("name").get<std::string>()};
    }
    static void to_json(nlohmann::json& jv, const UserResponse& obj) {
        jv["id"] = obj.id;
        jv["name"] = obj.name;
    }
};

template <>
struct clover2_http::serialization::json_traits<ChatMessage> {
    static ChatMessage from_json(const nlohmann::json& jv) {
        return {.user = jv.at("user").get<std::string>(),
                .body = jv.at("body").get<std::string>()};
    }

    static void to_json(nlohmann::json& jv, const ChatMessage& obj) {
        jv["user"] = obj.user;
        jv["body"] = obj.body;
    }
};

namespace c2 = clover2_http;

void handle_hello(c2::core::request_context /*ctx*/,
                  c2::endpoint::reply<EchoResponse> reply) {
    reply(EchoResponse{.echoed = "Hello from clover2_http!", .length = 27},
          200);
}

void handle_get_user(c2::core::request_context ctx,
                     c2::endpoint::reply<UserResponse> reply) {
    int id = std::stoi(ctx.path_params.at("id"));
    reply(UserResponse{.id = id, .name = "Alice"}, 200);
}

void handle_echo(c2::core::request_context /*ctx*/, EchoRequest req,
                 c2::endpoint::reply<EchoResponse> reply) {
    int len = static_cast<int>(req.text.size());
    reply(EchoResponse{.echoed = std::move(req.text), .length = len}, 200);
}

void handle_ws(
    std::shared_ptr<c2::transport::ws_session<ChatMessage>> session) {
    session->on_message(
        [](std::shared_ptr<c2::transport::ws_session<ChatMessage>> s,
           ChatMessage msg) {
            std::cout << "WS message from " << msg.user << ": " << msg.body
                      << '\n';
            s->write(
                ChatMessage{.user = "server", .body = "You said: " + msg.body});
        });

    session->on_close(
        [](std::shared_ptr<c2::transport::ws_session<ChatMessage>> /*s*/,
           int code) { std::cout << "WS closed, code=" << code << '\n'; });

    session->start_reading();
}

int main() {
    boost::asio::io_context io;

    c2::server srv(io, c2::core::simple_logger("example"));

    srv.get<void, EchoResponse>("/hello", handle_hello);
    srv.get<void, UserResponse>("/users/{id}", handle_get_user);
    srv.post<EchoRequest, EchoResponse>("/echo", handle_echo);
    srv.ws<ChatMessage>("/ws/chat", handle_ws);

    srv.listen("0.0.0.0", 8080);

    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code, int) { io.stop(); });

    io.run();
    return 0;
}
