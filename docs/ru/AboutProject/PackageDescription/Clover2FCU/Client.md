# Клиент и capabilities

## Создание клиента

Конструктор `clover2_fcu::client` принимает `clover2_fcu::context::node_interfaces` существующего узла и `client::settings`.

| Настройка | По умолчанию | Назначение |
| --- | --- | --- |
| `bridge_node()` | `/fcu_bridge` | Абсолютное имя bridge-узла |
| `wait_timeout()` | 10 с | Таймаут ожидания bridge и вызова `get_config` |
| `use_diagnostics()` | `true` | Включение клиентской диагностики |
| `require<T>()` / `require(name)` | Пустой список | Обязательные capabilities |

Метод `has<T>()` проверяет, поддерживает ли загруженный backend нужный интерфейс. Провайдер при такой проверке не создаётся. Если обязательный интерфейс отсутствует, конструктор выбрасывает `clover2_fcu::exception`.

```cpp
if (fcu->has<clover2_fcu::capability::odometry_interface>()) {
    auto odometry = fcu->odometry();
}
```

```{mermaid}
sequenceDiagram
    participant N as Узел-потребитель
    participant C as client
    participant B as /fcu_bridge

    N->>C: client(node_interfaces, settings)
    C->>B: Ожидание get_config
    C->>B: GetConfig
    B-->>C: protocol + plugin_params
    C->>C: Создание fcu callback group
    C->>C: Загрузка backend_base через pluginlib
    C->>C: initialize(context) и проверка require
    Note over C: Провайдеры создаются при первом вызове get()
```

## Телеметрия

`telemetry_interface<T>` хранит последний снимок данных. С ним можно работать несколькими способами:

- `last()`: вернуть копию последнего значения. До первого сообщения метод выбрасывает исключение.
- `has_data()`: проверить, приходило ли хотя бы одно значение.
- `last_valid(max_delay)`: проверить свежесть снимка. По умолчанию используется интервал 500 мс.
- `last_time()`: получить время последнего снимка.
- `on_update(callback)`: подписаться на новые значения.

```cpp
auto state = fcu->state();
if (state->last_valid(500ms)) {
    auto snapshot = state->last();
}

auto token = state->on_update(
    [](const clover2_fcu::data::system_state& snapshot) {
        // callback выполняется в потоке executor
    });
```

Метод `on_update()` возвращает RAII-токен. Храните его, пока нужна подписка. При уничтожении токена или вызове `token.release()` callback отписывается. Методы `last()` и `last_valid()` защищены мьютексом, поэтому их можно вызывать из других потоков.

```{mermaid}
sequenceDiagram
    participant T as Реализация транспорта
    participant P as Провайдер телеметрии
    participant U as Потребитель

    T->>P: Новое сообщение
    P->>P: update_data: сохранить снимок и время
    P-->>U: on_update(const T&)
    U->>P: last() или last_valid()
    P-->>U: Копия снимка / признак свежести
```

## Команды

Через `fcu.commands()` доступны асинхронные команды `arm`, `disarm`, `set_mode`, `takeoff`, `land` и `rtl`. Каждая команда принимает `result_callback` с сигнатурой `void(bool)`.

```cpp
fcu->commands()->arm([](bool ok) {
    if (!ok) {
        // Сервис недоступен или FCU отклонил команду
    }
});
```

```{important}
Callback со значением `true` подтверждает приём команды. Например, после успешного `land()` коптеру ещё нужно время на посадку. Момент касания земли отслеживайте по телеметрии в навигационной логике.
```

```{mermaid}
sequenceDiagram
    participant U as Потребитель
    participant P as commands
    participant T as Реализация транспорта
    participant F as FCU

    U->>P: arm(callback)
    P->>T: Запрос arm
    T->>F: Команда arm
    F-->>T: Подтверждение команды
    T-->>P: Результат запроса
    P-->>U: callback(true или false)
    Note over U,P: FCU подтвердил приём команды
```

## Параметры FCU

Интерфейс `parameters_interface` работает со значениями `rclcpp::ParameterValue`.

```cpp
rclcpp::ParameterValue value;
auto result = fcu->parameters()->get("SYS_AUTOSTART", value);

auto write_result = fcu->parameters()->set(
    "COM_OF_LOSS_T", rclcpp::ParameterValue(1.0));
```

Метод `get()` читает локальный кэш провайдера, а `list()` возвращает известные имена. Метод `set()` ждёт ответа FCU, поэтому его нельзя вызывать внутри callback FCU-группы. Результат имеет тип `clover2_fcu::result`: например, `success`, `rejected`, `timeout` или `unsupported`.

## Передача данных сенсоров

Через `estimator` можно получить `sensor_publisher<T>` для одометрии или оптического потока.

```cpp
auto estimator = fcu->get<clover2_fcu::capability::estimator_interface>();
auto flow = estimator->optical_flow_publisher();
if (flow) {
    flow.publish(sample);  // sample имеет тип data::optical_flow
}
```

Пустой handle показывает, что backend не может выделить канал. Вызов `publish()` для такого handle ничего не делает. Метод `id()` возвращает идентификатор, назначенный backend. Для одометрии используйте `odometry_publisher(estimator_source::vision)`. Доступные источники и число одновременных каналов зависят от backend.

## Потоки и callback group

Клиент создаёт отдельную callback-группу `MutuallyExclusive`. Подписки, клиенты сервисов и таймеры провайдеров привязываются к ней через `fcu::context`. Благодаря такой группе callback FCU выполняются по очереди даже при использовании `MultiThreadedExecutor`. Кэш провайдеров и снимки телеметрии защищены мьютексами.

Создание клиента, регистрацию режима и `parameters_interface::set()` выполняйте вне callback FCU-группы. Эти операции ждут ответа, который обрабатывает executor.

## Python

Пакет `clover2_fcu_py` предоставляет классы `Client`, `Settings`, `SystemState` и токен `Subscription`.

```python
from clover2_fcu import Client, Settings

settings = Settings()
settings.bridge_node = "/fcu_bridge"
settings.wait_timeout = 5.0
settings.require("system_state")

fcu = Client(settings)
state = fcu["system_state"]
subscription = state.on_update(lambda current: print(current))

if state.last_valid(0.5):
    print(state.last())
```

Метод `SystemState.last()` возвращает словарь с полями `connected`, `armed`, `in_air`, `failsafe` и `mode`. Для отписки вызовите `subscription.close()`. Храните объект `Subscription`, пока нужны уведомления. Внутренний ROS-узел клиента работает в фоновом потоке.
