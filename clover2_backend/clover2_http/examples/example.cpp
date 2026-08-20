// ./a.out
// curl http://0.0.0.0:8080/hello
// curl http://0.0.0.0:8080/users/42
// curl -X POST http://0.0.0.0:8080/echo -d '{"text":"hi"}'
// websocat ws://0.0.0.0:8080/ws/chat
// websocat ws://0.0.0.0:8080/ws/stream  (binary: websocat --binary ws://...)

#include <clover2_http/http/core/logger.hpp>
#include <clover2_http/http/middleware/cors.hpp>
#include <clover2_http/http/server.hpp>

#include <boost/asio/signal_set.hpp>

#include <cstdlib>
#include <iostream>
#include <string>

struct EchoRequest {
    std::string text;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EchoRequest, text)

struct EchoResponse {
    std::string echoed;
    int length;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EchoResponse, echoed, length)

struct UserResponse {
    int id;
    std::string name;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(UserResponse, id, name)

struct ChatMessage {
    std::string user;
    std::string body;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ChatMessage, user, body)

namespace c2 = clover2_http::http;

void handle_hello(c2::core::request_context /*ctx*/,
                  c2::endpoint::reply<EchoResponse>& reply) {
    reply(EchoResponse{.echoed = "Hello from clover2_http!", .length = 27},
          200);
}

void handle_get_user(c2::core::request_context ctx,
                     c2::endpoint::reply<UserResponse>& reply) {
    int id = std::stoi(ctx.path_params.at("id"));
    reply(UserResponse{.id = id, .name = "Alice"}, 200);
}

void handle_echo(c2::core::request_context /*ctx*/, EchoRequest req,
                 c2::endpoint::reply<EchoResponse>& reply) {
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

    // Raw binary handler via escape hatch
    session->raw()->on_binary(
        [](std::shared_ptr<c2::transport::base_ws_session> /*s*/,
           std::vector<uint8_t> data) {
            std::cout << "WS binary received: " << data.size() << " bytes\n";
        });

    session->start_reading();
}

void handle_raw_ws(std::shared_ptr<c2::transport::base_ws_session> session) {
    session->on_text([](std::shared_ptr<c2::transport::base_ws_session> s,
                        std::string text) {
        std::cout << "Raw WS text: " << text << '\n';
        s->write_text("Echo: " + text);
    });

    session->on_binary([](std::shared_ptr<c2::transport::base_ws_session> s,
                          std::vector<uint8_t> data) {
        std::cout << "Raw WS binary: " << data.size() << " bytes\n";
        s->write_binary(std::move(data));
    });

    session->on_close(
        [](std::shared_ptr<c2::transport::base_ws_session> /*s*/, int code) {
            std::cout << "Raw WS closed, code=" << code << '\n';
        });

    session->start_reading();
}

int main() {
    boost::asio::io_context io;

    c2::server srv(io, c2::core::simple_logger("example"));

    srv.use("/",
            [] { return std::make_unique<c2::middleware::cors>(); });

    srv.get<EchoResponse>("/hello", handle_hello);
    srv.get<UserResponse>("/users/{id}", handle_get_user);
    srv.post<EchoRequest, EchoResponse>("/echo", handle_echo);
    srv.ws<ChatMessage>("/ws/chat", handle_ws);
    srv.raw_ws("/ws/stream", handle_raw_ws);

    srv.listen("0.0.0.0", 8080);

    boost::asio::signal_set signals(io, SIGINT, SIGTERM);
    signals.async_wait([&](boost::system::error_code, int) { io.stop(); });

    io.run();
    return 0;
}
