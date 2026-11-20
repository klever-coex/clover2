# clover2_http

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Clover2HTTP/RosSupport
Clover2HTTP/MapServer
Clover2HTTP/SettingsServer
```

The `clover2_http` package provides an asynchronous HTTP server library built on [boost.Beast](https://www.boost.org/doc/libs/latest/libs/beast/doc/html/index.html), paired with a dedicated ROS 2 node to manage its execution. 
Within the `clover2` ecosystem, this package serves as the communication bridge between the web-based user interface and the underlying ROS 2 system.

## http-server-lib

The core of the library is the `clover2_http::http::server` class that provides the essential tools needed to build your server, allowing you to:

* Register HTTP request handlers to manage standard web requests.
* Register WebSocket connection handlers for real-time, bidirectional communication.
* Attach `middleware` to perform additional processing on incoming requests.

Full API Reference for `clover2_http::http::server`:

```{eval-rst}
.. doxygenclass:: clover2_http::http::server
    :members: server
```

### Usage Example

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

The above example demonstrates how to set up a basic server that responds to a specific `GET` request:
1. We register a handler for `GET` requests at the `/hello` path.
2. The server responds with the string `hello` and an HTTP status code of `200`.
3. The `listen()` method tells the server to accept connections from any network interface on port `3000`.
4. The `io.run()` call runs the asynchronous event loop.

## Plugins

You can extend the server's capabilities using the `pluginlib` framework. 
By inheriting from the `clover2_http::base_plugin` class and using the `clover2_http::plugin<T>` template wrapper, you can create modular extensions for your server.

Plugins are loaded automatically when the ROS 2 node starts; there is no need to manually register them in your configuration files.

Each plugin declares a list of `capabilities` (unique identifiers of its features). 
This list is returned to the frontend via a `GET /api/manifest` request, allowing the web interface to dynamically adapt to the server's available functions.

Available Plugins

| Plugin | Package | Capabilities | Purpose |
| --- | --- | --- | --- |
| {doc}`ros_support <Clover2HTTP/RosSupport>` | `clover2_http` | `nodes`, `topics`, `services` | REST API to interact with the ROS 2 graph and WebSocket streaming of topics in JSON format |
| {doc}`map_server <Clover2HTTP/MapServer>` | `clover2_ui/clover2_http_plugins` | `map` | REST CRUD API (Create, Read, Update, Delete) for managing ArUco map markers |
| {doc}`settings_server <Clover2HTTP/SettingsServer>` | `clover2_ui/clover2_http_plugins` | `settings` | Web-based editor for managing launch settings |

Plugins are categorized into two types:

- **Universal Plugins** are designed to work in any ROS 2 environment without dependency on specific `clover2` packages. You can find them in the `clover2_http` package under the `src/plugins` directory.
- **Project-Specific Plugins** are tailored to the `clover2` ecosystem and depend on other `clover2` packages (such as `clover2_map` or `clover2_ui`). They provide RESTful access to specialized project functionality and are located in the `clover2_ui/clover2_http_plugins` package.

### How to Create a New Plugin

To extend the server with your own custom functionality, follow these steps to implement a new plugin:
1. Create a new class that inherits from `clover2_http::plugin<T>`, where `<T>` is the name of your plugin class.
2. Set the static parameters `k_name` (a unique identifier) and `k_version` (the plugin version).
3. Override `on_initialize()` method to register your HTTP or WebSocket paths. You can use `m_server->get()`, `m_server->post()`, `m_server->put()`, `m_server->del()` methods.
4. Override the `capabilities()` method to return a list of strings representing the features your plugin provides.
5. Use the `PLUGINLIB_EXPORT_CLASS` macro to make your plugin discoverable.
6. Add your plugin's description to the `plugins.xml` file to complete the integration.

### Minimal Plugin Example

The following example demonstrates a simple `ping` plugin. It declares a `ping` capability and responds to `GET /api/ping` with a JSON object.

```c++
// src/plugins/ping_plugin.cpp
#include <clover2_http/plugin.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace clover2_http::plugins {

namespace http = clover2_http::http;

// Define the response structure. 
// Using a struct with 
struct pong {
    bool success = true;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(pong, success)

class ping_plugin : public clover2_http::plugin<ping_plugin> {
public:
    static constexpr std::string_view k_name = "ping_plugin";
    static constexpr int k_version = 1;

protected:
    // Called once at server startup: this is where you register your paths.
    void on_initialize() override {
        m_server->get<pong>(
            "/api/ping",
            [](http::core::request_context /*ctx*/,
               http::endpoint::deferred_reply<pong> reply) {
                reply(pong{}, 200);
            });
    }

    // These capabilities will be listed in the GET /api/manifest.
    std::vector<std::string> capabilities() const override {
        return {"ping"};
    }
};

}  // namespace clover2_http::plugins

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(clover2_http::plugins::ping_plugin,
                       clover2_http::base_plugin)
```

Key components of the above example:
* `k_name` and `k_version` identify the plugin to the system.
* `on_initialize()` registers a `GET` handler for the `/api/ping` endpoint.
* `reply(pong{}, 200)` call automatically serializes our pong struct into a JSON response and attaches the `200` status code.
* By returning `{"ping"}` in `capabilities()`, the web interface knows that the server supports the `ping` functionality via the `api/manifest`.
  
### Plugin Description and Build

To ensure `pluginlib` can discover and load your new plugin, you must define it in an XML configuration file and update your build scripts.

Create a `plugins.xml` file in your package's root directory. This file maps the library to the specific class you implemented.

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

Attributes:
- `path` specifies the name of the CMake library target created during the build process.
- `type` is the fully qualified name of your plugin class, including its namespace.
- `base_class_type` is the base class that your plugin inherits from.

Add the following commands to your CMakeLists.txt to compile the plugin as a shared library and export the plugin description file so ROS 2 can locate it:

```cmake
ament_auto_add_library(ping_plugin SHARED DIRECTORY "src")
pluginlib_export_plugin_description_file(clover2_http plugins.xml)
```

Once you have built and installed your package, the plugin will be automatically detected and loaded by the `clover2_http` node upon startup.
The plugin's declared capabilities will immediately appear in the `GET /api/manifest` response.
