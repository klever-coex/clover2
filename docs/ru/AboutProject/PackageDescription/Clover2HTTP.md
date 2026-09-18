# clover2_http

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Clover2HTTP/RosSupport
Clover2HTTP/MapServer
Clover2HTTP/SettingsServer
```

Данный пакет содержит библиотеку асинхронного веб сервера на базе [boost.Beast](https://www.boost.org/doc/libs/latest/libs/beast/doc/html/index.html) и ROS2 ноду запускающую http сервер. В проекте `clover2` используется для работы web интерфейса

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

srv.get<std::string>("/hello", [](http::core::request_context ctx,
                            http::endpoint::deferred_reply<std::string> reply ){
                                reply("hello", 200);
                            });

srv.listen("0.0.0.0", 3000);
io.run();
```

В данном примере регистрируется путь `/hello` для `GET` запросов. Ответом всегда будет строчка "hello". Сервер запускается на 3000 порту.

## Плагины

Функциональность сервера расширяется плагинами `pluginlib` с базовым классом `clover2_http::base_plugin` (обёртка `clover2_http::plugin<T>`). Все объявленные классы загружаются автоматически при старте ноды — отдельная регистрация в конфигурации не требуется.

Каждый плагин объявляет список `capabilities` — он попадает в `GET /api/manifest` и используется фронтендом для определения функционала.

Все плагины репозитория:

| Плагин | Пакет | Capabilities | Назначение |
| --- | --- | --- | --- |
| {doc}`ros_support <Clover2HTTP/RosSupport>` | `clover2_http` | `nodes`, `topics`, `services` | REST API графа ROS 2 и WebSocket-стриминг топиков в JSON |
| {doc}`map_server <Clover2HTTP/MapServer>` | `clover2_ui/clover2_http_plugins` | `map` | REST CRUD маркеров ArUco-карты |
| {doc}`settings_server <Clover2HTTP/SettingsServer>` | `clover2_ui/clover2_http_plugins` | `settings` | Веб-редактор настроек запуска |

Плагины делятся на два типа:

- **Универсальные** — работают в любом ROS2 окружении и не зависят от пакетов проекта. Живут в самом `clover2_http` (`src/plugins`).
- **Проектные** — зависят от пакетов clover2 (`clover2_map`, `clover2_ui` и тд) и оборачивают их логику в REST. Живут в пакете `clover2_ui/clover2_http_plugins`.

Новый плагин добавляется так: класс наследуется от `clover2_http::plugin<T>`, определяется `k_name`/`k_version`, переопределяются `on_initialize()` (регистрация маршрутов через `m_server->get/post/put/del`) и `capabilities()`; класс экспортируется макросом `PLUGINLIB_EXPORT_CLASS` и описывается в `plugins.xml` пакета.

### Минимальный пример плагина

Плагин, отвечающий на `GET /api/ping` и объявляющий capability `ping`:

```c++
// src/plugins/ping_plugin.cpp
#include <clover2_http/plugin.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace clover2_http::plugins {

namespace http = clover2_http::http;

// Тип ответа. Не обязательно выносить в отдельную структуру, но так удобнее.
struct pong {
    bool success = true;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(pong, success)

class ping_plugin : public clover2_http::plugin<ping_plugin> {
public:
    static constexpr std::string_view k_name = "ping_plugin";
    static constexpr int k_version = 1;

protected:
    // Вызывается один раз при старте сервера: регистрируем маршруты.
    void on_initialize() override {
        m_server->get<pong>(
            "/api/ping",
            [](http::core::request_context /*ctx*/,
               http::endpoint::deferred_reply<pong> reply) {
                reply(pong{}, 200);
            });
    }

    // Попадает в GET /api/manifest: фронтенд считает функциональность
    // доступной, только если список не пуст.
    std::vector<std::string> capabilities() const override {
        return {"ping"};
    }
};

}  // namespace clover2_http::plugins

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_http::plugins::ping_plugin,
                       clover2_http::base_plugin)
```

Классу нужен Export-файл `plugins.xml` в корне пакета (`library path` — имя cmake-таргета библиотеки):

```xml
<class_libraries>
    <library path="ping_plugin">
        <class
            name="ping_plugin"
            type="clover2_http::plugins::ping_plugin"
            base_class_type="clover2_http::base_plugin"
        />
    </library>
</class_libraries>
```

```cmake
ament_auto_add_library(ping_plugin SHARED DIRECTORY "src")
pluginlib_export_plugin_description_file(clover2_http plugins.xml)
```

После сборки плагин подхватится автоматически и появится в `/api/manifest`.
