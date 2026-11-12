# clover2_fcu

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Clover2FCU/Client
Clover2FCU/Modes
Clover2FCU/MavrosBackend
Clover2FCU/BackendDevelopment
```

`clover2_fcu` даёт единый API для работы с полётным контроллером. Клиент добавляется в узел ROS 2 как обычный C++-класс. При создании он получает настройки у bridge и загружает backend нужного протокола через `pluginlib`. Смена протокола не требует правок в пользовательском коде.

## Состав

| Пакет | Назначение |
| --- | --- |
| `clover2_fcu` | C++-клиент, bridge-узел, интерфейсы capabilities и базовый класс режимов `mode_base` |
| `clover2_fcu_msgs` | Сообщения и сервисы `FcuConfig`, `GetConfig`, `RegisterMode`, `ActivateMode`, `ModeState` |
| `clover2_fcu_mavros` | Реализация capabilities через сервисы и топики MAVROS |
| `clover2_fcu_py` | Python-биндинги клиента с ограниченным набором функций |

## Общая схема

```{mermaid}
flowchart LR
    subgraph APP["Пользовательская нода"]
        C["client"]
        M["mode_base"]
        CAP["capabilities: телеметрия; команды; параметры; setpoint"]
        C --> CAP
        M --> C
    end

    B["/fcu_bridge<br/>конфигурация и реестр режимов"]
    BE["backend_base<br/>плагин протокола"]
    MAV["mavros"]
    DDS["DDS"]
    FCU["Полётный контроллер"]

    C <-->|"GetConfig"| B
    M <-->|"режимы и activation lease"| B
    C -->|"pluginlib"| BE
    CAP <-->|"данные и команды"| BE
    BE <-.-> MAV
    BE <-.-> DDS
    MAV <-.-> FCU
    DDS <-.-> FCU
```

Bridge хранит конфигурацию и ведёт реестр пользовательских режимов. Телеметрия и команды проходят через backend напрямую, без промежуточных преобразований в bridge. MAVROS и DDS показаны как примеры транспортов. Новый backend можно подключить через тот же интерфейс `pluginlib`.

## Пример C++

Передайте клиенту интерфейсы существующего узла. При создании клиент синхронно запрашивает конфигурацию через `/fcu_bridge/get_config`.

```cpp
#include <chrono>
#include <memory>

#include <clover2_fcu/client.hpp>
#include <rclcpp/rclcpp.hpp>

using namespace std::chrono_literals;

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    auto node = std::make_shared<rclcpp::Node>("fcu_example");

    auto settings = clover2_fcu::client::settings{}
                        .wait_timeout(5s)
                        .require<clover2_fcu::capability::system_state_interface>();
    auto fcu = std::make_shared<clover2_fcu::client>(
        clover2_fcu::context::node_interfaces(*node), settings);

    auto state = fcu->state();
    auto subscription = state->on_update(
        [](const clover2_fcu::data::system_state& current) {
            // Обработка нового снимка состояния
        });

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
```

Храните `subscription`, пока нужны уведомления. Для разового чтения вызовите `state->last()` после проверки через `state->has_data()` или `state->last_valid()`.

## Capabilities

```{important}
Capabilities описывают отдельные функции полётного контроллера. Backend может поддерживать только часть из них, поэтому перед использованием проверяйте нужный интерфейс через `has<Capability>()`.
```

| Имя | Назначение |
| --- | --- |
| `system_state` | Подключение, armed, нахождение в воздухе, failsafe и текущий режим |
| `odometry` | Локальная позиция, ориентация и скорости |
| `battery` | Телеметрия основной батареи и доступ к источникам по идентификатору |
| `commands` | Команды `arm`, `disarm`, `set_mode`, `takeoff`, `land`, `rtl` с callback результата |
| `parameters` | Чтение и запись параметров FCU, список известных имён |
| `estimator` | Установка начала координат, передача одометрии и оптического потока, сброс estimator |
| `setpoint_position` | Позиционные setpoint |
| `setpoint_velocity` | Setpoint скорости |

Клиент создаёт провайдер при первом вызове `get<T>()` и сохраняет его в кэше. Метод `has<T>()` проверяет поддержку capability. Чтобы клиент требовал нужный интерфейс при запуске, добавьте его через `settings.require<T>()`.
