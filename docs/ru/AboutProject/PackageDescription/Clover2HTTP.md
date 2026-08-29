# clover2_http

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Clover2HTTP/RosSupport
Clover2HTTP/MapServer
```

Данный пакет соддержит библиотеку асинхронного веб сервера на базе [boost.Beast](https://www.boost.org/doc/libs/latest/libs/beast/doc/html/index.html) и ROS2 ноду запускающую http сервер. В проекте clover2 используется для работы web интерфейса

## http-server-lib

Основной класс библиотеки - `clover2_http::http::server`. Через него происходит регистрация путей для http и WebSocket запросов и `middleware`.

```{eval-rst}
.. doxygenclass:: clover2_http::http::server
    :members: server
```

### Пример использования

```c++
using http = clover2_http::http;

boost::asio::io_context io;
http::server srv(io, http::core::simple_logger("example"));

srv.get<std::string>("/hello", [](c2::core::request_context ctx,
                            http::endpoint::deferred_reply<std::string> reply ){
                                reply("hello", 200);
                            });

srv.listen("0.0.0.0", 3000);
io.run();
```

В данном примере регистрируется путь `/hello` для `GET` запросов. Ответом всегда будет строчка "hello". Сервер запускается на 3000 порту.
