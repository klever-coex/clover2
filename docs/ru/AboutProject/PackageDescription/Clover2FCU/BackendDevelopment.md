# Разработка бэкенда

Backend переводит вызовы `clover2_fcu` в команды конкретного протокола FCU. Клиент и bridge загружают его через `pluginlib` по имени протокола.

## Жизненный цикл

Новый backend наследует `clover2_fcu::backend::backend_base` и реализует следующие методы:

- `protocol()`: имя протокола;
- `do_initialize()`: инициализация после сохранения `fcu::context`;
- `register_mode()` и `unregister_mode()`: нативные режимы FCU, если протокол их поддерживает.

В конструкторе зарегистрируйте builder для каждой capability через `add<T>(builder)`. Builder принимает `const context&` и возвращает `std::shared_ptr` провайдера нужного интерфейса. Метод `capabilities()` собирает список из зарегистрированных builder.

```{mermaid}
sequenceDiagram
    participant C as client или bridge
    participant L as pluginlib
    participant B as backend_base
    participant P as Провайдер capability

    C->>L: createSharedInstance(protocol)
    L-->>C: Экземпляр бэкенда
    C->>B: Проверка protocol() и initialize(context)
    B->>B: Сохранить context, вызвать do_initialize()
    C->>B: get(capability)
    B->>P: builder(context)
    P-->>C: shared_ptr провайдера
    Note over C: client хранит провайдер в своём кэше
```

В `context` доступны logger, clock, интерфейсы узла, callback-группа FCU, имя bridge и `plugin_params`. Создавайте ROS-сущности через `subscribe()`, `publisher()`, `client()` и `timer()`: эти методы сами выбирают нужную callback-группу. Параметры capability можно получить через `ctx.params(capability_name)`.

## Пример backend

В примере ниже `my_system_state` и `my_commands` реализуют соответствующие интерфейсы. Их конструкторы и работа с протоколом зависят от выбранного транспорта.

```cpp
#include <memory>
#include <string>

#include <clover2_fcu/backend/backend_base.hpp>

class my_backend : public clover2_fcu::backend::backend_base {
public:
    my_backend() {
        add<clover2_fcu::capability::system_state_interface>(
            [](const clover2_fcu::context& ctx) {
                return std::make_shared<my_system_state>(ctx);
            });
        add<clover2_fcu::capability::commands_interface>(
            [](const clover2_fcu::context& ctx) {
                return std::make_shared<my_commands>(ctx);
            });
    }

    std::string protocol() const override { return "my_protocol"; }

protected:
    void do_initialize() override {}
};
```

Для транспорта без нативной регистрации режимов метод `register_mode()` переопределять не нужно. Базовая реализация возвращает `std::nullopt`, а bridge ведёт собственный реестр.

## Экспорт pluginlib

После определения класса добавьте экспорт.

```cpp
#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(my_backend, clover2_fcu::backend::backend_base)
```

Затем создайте XML-описание, например `my_backend_plugins.xml`.

```xml
<library path="clover2_fcu_my_backend">
  <class name="my_protocol"
         type="my_backend"
         base_class_type="clover2_fcu::backend::backend_base">
    <description>Example FCU backend</description>
  </class>
</library>
```

В `CMakeLists.txt` соберите shared library и зарегистрируйте описание.

```cmake
ament_auto_add_library(clover2_fcu_my_backend SHARED DIRECTORY "src")
pluginlib_export_plugin_description_file(clover2_fcu my_backend_plugins.xml)
```

```{important}
Атрибут `class name` в XML, результат `protocol()` и ROS-параметр `protocol` должны совпадать. В примере используется `my_protocol`. При разных именах клиент и bridge завершат загрузку с ошибкой.
```

ROS-параметр `protocol` выбирает плагин. Строковый параметр `plugin_params` содержит YAML-конфигурацию backend. Клиент получает оба значения в ответе `GetConfig`. Дополнительная регистрация capability вне builder не нужна.
