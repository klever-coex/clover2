# clover2_common

Данный пакет содержит общие для всего проекта классы и инструменты: обёртки над стандартными нодами ROS 2, интерфейсы диагностики и наблюдения за параметрами, а также вспомогательные утилиты.

## Классы обёртки стандартных C++ нод

### clover2_common::node (node.cpp)

Обёртка над стандартным `rclcpp::Node`. Добавляет к обычной ноде две возможности: систему диагностики и наблюдение за параметрами.

**Методы:**

- `declare_and_watch_parameter<ParameterT>(name, default_value, cb, ...)` — объявляет параметр и подписывает на его изменения колбэк `cb`, который вызывается при изменении значения. Дополнительно принимает описание, ограничения, флаг `read_only` и `ignore_override`.
- `get_node_diagnostics_interface()` — возвращает интерфейс диагностики ноды.
- `get_node_parameters_watcher_interface()` — возвращает интерфейс наблюдения за параметрами.

### clover2_common::lifecycle_node (lifecycle_node.cpp)

Обёртка над `rclcpp_lifecycle::LifecycleNode`. Как и `node`, добавляет диагностику и наблюдение за параметрами, а также:

- автоматическую инициализацию: при запуске нода сама вызывает `configure()` и `activate()` — управляется параметром `autostart` (по умолчанию `true`);
- диагностическую задачу `/system/lifecycle_state`, которая сообщает текущее состояние жизненного цикла ноды.

**Методы** те же, что у `node`: `declare_and_watch_parameter`, `get_node_diagnostics_interface`, `get_node_parameters_watcher_interface`.

### clover2_common::node_runtime (node_runtime.cpp)

Запускает ноду `clover2_common::node` в отдельном потоке с собственным `SingleThreadedExecutor`. Удобен, когда ноду нужно создать и «крутить» без ручного управления потоками.

**Методы:**

- `start()` — запускает ноду: стартует поток, в котором крутится executor.
- `stop()` — останавливает executor и дожидается завершения потока.
- `get_node()` — возвращает указатель на ноду.
- `get_node_context()` — возвращает контекст ноды.

### clover2_common::node_context (node_context.hpp)

Агрегирует все интерфейсы ноды в одном объекте: стандартные интерфейсы ROS 2 (base, graph, logging, parameters, services, topics, timers и остальные), а также собственные — `NodeDiagnosticsInterface` и `NodeParametersWatcherInterface`.

**Методы:**

- `get_logger()` — возвращает логгер ноды.

### clover2_common::executor (executor.cpp)

`MultiThreadedExecutor` с именованием потоков, чтобы их было проще находить при отладке.

## Интерфейсы ноды

### NodeDiagnosticsInterface / NodeDiagnostics

Обёртка над `diagnostic_updater`, которая хранит диагностические задачи по их типу и управляет ими.

**Методы:**

- `add<T>(args...)` — добавляет диагностическую задачу типа `T`.
- `get<T>()` — возвращает зарегистрированную задачу типа `T`.
- `remove<T>()` — удаляет задачу типа `T`.
- `force_update()` — принудительно обновляет все диагностики.

### NodeParametersWatcherInterface / NodeParametersWatcher

Наблюдение за параметрами ноды: колбэк вызывается автоматически при изменении значения параметра.

**Методы:**

- `declare_and_watch_parameter(name, default_value, cb, descriptor, ignore_override)` — объявляет параметр и подписывает колбэк на его изменения.
- `undeclare_watcher_parameters()` — снимает объявление всех отслеживаемых параметров.

## Вспомогательные утилиты

### util::parameter (parameter.hpp)

Помощники для безопасного объявления и чтения параметров:

- `declare_parameter_if_not_declared(node, name, default_value, descriptor)` — объявляет параметр, только если он ещё не объявлен.
- `safe_declare_and_get(node, name, default_value, read_value, descriptor)` — объявляет параметр (если нужно) и сразу читает его значение.
- `declare_and_watch_parameter(watcher, name, default_value, cb, ...)` — объявляет и подписывает параметр, формируя дескриптор из описания и ограничений.

### util::time_buffer (time_buffer.hpp)

Буфер с окном по времени: хранит значения вместе с метками времени и автоматически удаляет записи, которые старше заданного окна.

**Методы:**

- `add(timestamp, data)` — добавляет значение или вектор значений с меткой времени.
- `prune(current_time)` — удаляет устаревшие записи.
- `front()` / `back()` — первое / последнее значение.
- `size()` / `empty()` / `clear()` — размер, проверка пустоты, очистка.
- `windowSize()` — размер окна хранения.

### rclcpp_trails (rclcpp_trails.hpp)

Расширение стандартного `rclcpp`: упрощённый `rclcpp::create_client<ServiceT>(node, service_name, qos, group)`, который работает напрямую с нодой и сам извлекает нужные интерфейсы.
