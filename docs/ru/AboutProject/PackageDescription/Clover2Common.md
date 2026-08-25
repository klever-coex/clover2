# clover2_common

Данный пакет содержит общие для всего проекта классы и инструменты: обёртки над стандартными нодами ROS 2, интерфейсы диагностики и наблюдения за параметрами, а также вспомогательные утилиты.

## Классы обёртки стандартных C++ нод

Классы-обёртки над стандартными C++ нодами — требуется для унификации функционала который часто используется.
`node` и `lifecycle_node` имеют одинаковые дополнительные методы.

- `declare_and_watch_parameter<ParameterT>(name, default_value, cb, ...)` — повторяет сигнатуру объявления параметра и регистрирует функцию на обновление:

```cpp
    declare_and_watch_parameter<std::string>(
        "frame_id", "base_link",
        [this](const rclcpp::Parameter& p) { m_frame_id = p.as_string(); },
        "Tracking target");
```

```{eval-rst}
.. doxygenclass:: clover2_common::node
    :members:
```

<br>

### `clover2_common::node_context`

Агрегирует все интерфейсы ноды в одном объекте: стандартные интерфейсы ROS 2, а также специфические для проекта `NodeDiagnosticsInterface` и `NodeParametersWatcherInterface`. (Аналог `rclcpp::node_interfaces::NodeInterfaces`)

```{eval-rst}
.. doxygenclass:: clover2_common::node_context
    :members:
```

<br>

### `clover2_common::node_runtime`

Запускает ноду `clover2_common::node` в отдельном потоке с собственным `SingleThreadedExecutor`. Необходим для работы программ с несоколькими циклами выполнения (`Boost.Asio` или `cpptui`)

```{eval-rst}
.. doxygenclass:: clover2_common::node_runtime
    :members:
```

## Собственные интерфейсы ноды

<br>

### NodeDiagnosticsInterface / NodeDiagnostics

Обёртка над `diagnostic_updater`, которая хранит диагностические задачи по типу и обеспечивает типизированный доступ.

```cpp
get_node_diagnostics_interface()->add<diagnostics::lifecycle_state_task>();
...
get_node_diagnostics_interface()
    ->get<diagnostics::lifecycle_state_task>()
    .set_state_getter([this]() { return get_current_state(); });
```

```{eval-rst}
.. doxygeninterface:: clover2_common::node_interfaces::NodeDiagnosticsInterface
    :members:
```

<br>

### NodeParametersWatcherInterface / NodeParametersWatcher

Наблюдение за параметрами ноды: колбэк вызывается автоматически при изменении значения параметра.

```{eval-rst}
.. doxygeninterface:: clover2_common::node_interfaces::NodeParametersWatcherInterface
    :members:
```
