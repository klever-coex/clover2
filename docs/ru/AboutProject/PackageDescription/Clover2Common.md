# clover2_common

Пакет `clover2_common` содержит общие для проекта классы и вспомогательные инструменты: обертки над стандартными классами ROS 2, интерфейсы для диагностики и отслеживания параметров нод, а также дополнительные утилиты.

## Классы обертки стандартных C++ нод

Классы-обертки расширяют стандартные классы C++ нод ROS 2 дополнительными методами. Это позволяет унифицировать работу с нодами и повторно использовать общий функционал в разных частях проекта.


Классы `node` и `lifecycle_node` предоставляют одинаковый набор дополнительных методов.

Например, метод `declare_and_watch_parameter<ParameterT>(name, default_value, cb, ...)` позволяет объявить параметр ноды и зарегистрировать функцию обратного вызова, которая будет выполняться при изменении значения этого параметра.

Пример объявления параметра `frame_id` со значением по умолчанию `base_link`:

```cpp
    declare_and_watch_parameter<std::string>(
        "frame_id", "base_link",
        [this](const rclcpp::Parameter& p) { m_frame_id = p.as_string(); },
        "Tracking target");
```

В этом примере при изменении параметра `frame_id` функция обратного вызова обновляет значение переменной `m_frame_id`.

Полный список методов класса node:

```{eval-rst}
.. doxygenclass:: clover2_common::node
    :members:
```

<br>

### `clover2_common::node_context`

Класс `node_context` объединяет интерфейсы ноды в одном объекте. Он предоставляет доступ как к стандартным интерфейсам ROS 2, так и к интерфейсам проекта:
* `NodeDiagnosticsInterface` — для работы с диагностическими задачами;
* `NodeParametersWatcherInterface` — для отслеживания изменений параметров.

Класс `node_context` выполняет функцию, аналогичную `rclcpp::node_interfaces::NodeInterfaces`.

```{eval-rst}
.. doxygenclass:: clover2_common::node_context
    :members:
```

<br>

### `clover2_common::node_runtime`

Класс `node_runtime` запускает  ноду `clover2_common::node` в отдельном потоке с собственным исполнителем `SingleThreadedExecutor`. Необходим для работы программ с несколькими циклами выполнения на базе`Boost.Asio` или `cpptui`.

Полный список методов класса:

```{eval-rst}
.. doxygenclass:: clover2_common::node_runtime
    :members:
```

## Собственные интерфейсы ноды

Пакет `clover2_common` предоставляет собственные интерфейсы для работы с диагностикой и отслеживания изменений параметров ноды.

### NodeDiagnosticsInterface / NodeDiagnostics

Интерфейс `NodeDiagnosticsInterface` предоставляет доступ к диагностическим задачам ноды. Он основан на библиотеке `diagnostic_updater`, которая хранит диагностические задачи и обеспечивает типизированный доступ.

Например, можно добавить задачу `lifecycle_state_task`, а затем получить ее экземпляр и задать функцию, которая возвращает текущее состояние ноды:

```cpp
get_node_diagnostics_interface()->add<diagnostics::lifecycle_state_task>();
...
get_node_diagnostics_interface()
    ->get<diagnostics::lifecycle_state_task>()
    .set_state_getter([this]() { return get_current_state(); });
```

В этом примере функция `get_current_state()` используется для получения текущего состояния ноды.

Описание интерфейса:

```{eval-rst}
.. doxygeninterface:: clover2_common::node_interfaces::NodeDiagnosticsInterface
    :members:
```

<br>

### NodeParametersWatcherInterface / NodeParametersWatcher

Интерфейс `NodeParametersWatcherInterface` позволяет отслеживать изменения параметров ноды.
При изменении значения параметра автоматически вызывается зарегистрированная функция обратного вызова (callback).

Описание интерфейса:

```{eval-rst}
.. doxygeninterface:: clover2_common::node_interfaces::NodeParametersWatcherInterface
    :members:
```

<br>

Интерфейсы `NodeDiagnosticsInterface` и `NodeParametersWatcherInterface` уже реализованы в пакете `clover2_common` и доступны через объект ноды. 
