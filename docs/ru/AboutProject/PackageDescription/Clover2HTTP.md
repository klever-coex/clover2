# clover2_http

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Clover2HTTP/RosSupport
Clover2HTTP/MapServer
Clover2HTTP/SettingsServer
```

Пакет `clover2_http` содержит библиотеку асинхронного HTTP-сервера на базе [boost.Beast](https://www.boost.org/doc/libs/latest/libs/beast/doc/html/index.html) и ROS 2 ноду для его запуска. В проекте `clover2` пакет используется для организации взаимодействия веб-интерфейса с ROS 2. 

## http-server-lib

Основной класс библиотеки — `clover2_http::http::server`. С помощью этого класса можно:
* регистрировать обработчики HTTP-запросов;
* регистрировать обработчики WebSocket-соединений;
* подключать` middleware` для дополнительной обработки запросов.

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

В этом примере:
1. Регистрируется обработчик `GET`-запросов по адресу `/hello`.
2. При получении запроса сервер отправляет строку `hello` с HTTP-кодом ответа `200`.
3. Метод `listen()` запускает прослушивание входящих подключений на порту `3000`. Адрес `0.0.0.0` позволяет принимать подключения через все сетевые интерфейсы.
4. Метод `io.run()` запускает обработку асинхронных операций.

## Плагины

Функциональность HTTP-сервера можно расширять с помощью плагинов `pluginlib` с базовым классом `clover2_http::base_plugin`. Для создания плагинов используется шаблонная обертка `clover2_http::plugin<T>`. 

Плагины загружаются автоматически при запуске ROS 2 ноды — отдельная регистрация в конфигурации не требуется.

Каждый плагин объявляет список `capabilities` — идентификаторов доступных возможностей. Этот список передается в ответе на запрос `GET /api/manifest` и используется фронтендом для определения
какие функции поддерживает сервер.

Все плагины репозитория:

| Плагин | Пакет | Capabilities | Назначение |
| --- | --- | --- | --- |
| {doc}`ros_support <Clover2HTTP/RosSupport>` | `clover2_http` | `nodes`, `topics`, `services` | REST API для работы с графом ROS 2 и WebSocket-стриминг топиков в формате JSON |
| {doc}`map_server <Clover2HTTP/MapServer>` | `clover2_ui/clover2_http_plugins` | `map` | REST CRUD для создания, чтения, изменения и удаления маркеров ArUco-карты |
| {doc}`settings_server <Clover2HTTP/SettingsServer>` | `clover2_ui/clover2_http_plugins` | `settings` | Веб-редактор настроек запуска |

Плагины делятся на два типа:

- **Универсальные** — работают в любом окружении ROS 2 и не зависят от пакетов проекта `clover2`. Размещаются в пакете  `clover2_http` в каталоге `src/plugins`.
- **Проектные** — зависят от пакетов проекта `clover2` (например, `clover2_map`, `clover2_ui`) и предоставляют доступ к их функциональности через REST. Размещаются в пакете `clover2_ui/clover2_http_plugins`.

### Создание нового плагина

Чтобы добавить новый плагин:

1.	Создайте класс, унаследованный от `clover2_http::plugin<T>`, где `<T>` — создаваемый класс плагина.
2.	Определите статические параметры `k_name` и `k_version`.
3.	Переопределите метод `on_initialize()`. В нем зарегистрируйте обработчики HTTP- и WebSocket-запросов с помощью методов `m_server->get()`, `m_server->post()`, `m_server->put()` и `m_server->del()`.
4.	Переопределите метод `capabilities()`, чтобы объявить возможности плагина.
5.	Экспортируйте класс с помощью макроса `PLUGINLIB_EXPORT_CLASS`.
6.	Добавьте описание плагина в файл `plugins.xml`.

### Минимальный пример плагина

Рассмотрим пример плагина, отвечающего на `GET /api/ping` и объявляющего capability `ping`:

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

    // Попадает в GET /api/manifest.
    std::vector<std::string> capabilities() const override {
        return {"ping"};
    }
};

}  // namespace clover2_http::plugins

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_http::plugins::ping_plugin,
                       clover2_http::base_plugin)
```

В этом примере:
* `k_name` задает имя плагина.
* `k_version` задает версию плагина.
* `on_initialize()` регистрирует обработчик `GET`-запросов по адресу `/api/ping`.
* `reply(pong{}, 200)` отправляет JSON-ответ с HTTP-кодом `200`.
* `capabilities()` объявляет возможность `ping`, которая передается веб-интерфейсу через `api/manifest`.


### Описание и сборка плагина

Для обнаружения плагина `pluginlib` необходимо создать файл `plugins.xml` в корне пакета:

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

В атрибуте `path` указывается имя CMake-цели библиотеки, а в атрибуте `type` — полное имя класса плагина с пространством имен.

Добавьте в CMakeLists.txt создание библиотеки и экспорт описания плагина:


```cmake
ament_auto_add_library(ping_plugin SHARED DIRECTORY "src")
pluginlib_export_plugin_description_file(clover2_http plugins.xml)
```

После сборки и установки пакета плагин станет доступен для автоматической загрузки при запуске ноды. Его возможности появятся в `/api/manifest`.
