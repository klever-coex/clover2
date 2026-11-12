# Пользовательские режимы

Для своего режима полёта унаследуйте класс `clover2_fcu::mode_base`. В нём можно публиковать position- или velocity-setpoint. Bridge хранит список режимов и выдаёт activation lease активному режиму. Отправкой setpoint и переключением FCU занимается сам режим.

## ROS API bridge

При имени bridge `/fcu_bridge` доступны следующие интерфейсы:

| Интерфейс | Тип | Назначение |
| --- | --- | --- |
| `/fcu_bridge/get_config` | `clover2_fcu_msgs/srv/GetConfig` | Получение `protocol` и YAML-строки `plugin_params` |
| `/fcu_bridge/register_mode` | `clover2_fcu_msgs/srv/RegisterMode` | Регистрация уникального имени и получение `mode_id` |
| `/fcu_bridge/activate_mode` | `clover2_fcu_msgs/srv/ActivateMode` | Активация по имени; пустое имя снимает текущий lease |
| `/fcu_bridge/mode_state` | `clover2_fcu_msgs/msg/ModeState` | Текущие `mode_id` и имя; QoS `transient_local` |
| `/fcu_bridge/bond` | `bond` | Проверка работы узла зарегистрированного режима |

Имена режимов должны быть уникальными. Значение `mode_id == 0` показывает, что сейчас нет активного режима.

## Наследование mode_base

При создании `mode_base` регистрирует режим через блокирующий вызов `register_mode`, подписывается на `mode_state` и запускает bond heartbeat с периодом 0,1 с. Настройки передаются через `mode_base::settings`.

```cpp
class hover_mode : public clover2_fcu::mode_base {
public:
    explicit hover_mode(clover2_fcu::client::shared_ptr fcu)
        : clover2_fcu::mode_base(
              fcu, settings{}.with_name("hover").with_update_rate(30.f)) {}

protected:
    void on_activate() override {
        // Подготовка контроллера
    }

    void update_setpoint(double dt_s) override {
        position()->update(Eigen::Vector3d(0.0, 0.0, 1.0));
    }

    void on_deactivate() override {
        // Очистка состояния контроллера
    }
};
```

Методы `position()` и `velocity()` возвращают кэшированные capabilities клиента. Отправляйте setpoint из `update_setpoint()` активного режима. На один узел предполагается один `mode_base`.

## Активация

Для активации вызовите `activate_mode` с именем зарегистрированного режима. После получения подходящего `mode_state` режим выполняет `on_activate()`. Если FCU находится в другом режиме, `mode_base` запрашивает `OFFBOARD` через `commands()->set_mode()`. Отправка setpoint начинается после подтверждения `OFFBOARD` в телеметрии.

```{mermaid}
sequenceDiagram
    participant N as Потребитель
    participant B as /fcu_bridge
    participant M as mode_base
    participant F as FCU через commands

    M->>B: RegisterMode(name)
    B-->>M: mode_id
    M->>B: bond heartbeat каждые 0,1 с
    N->>B: ActivateMode(name)
    B-->>M: mode_state с mode_id режима
    M->>M: on_activate()
    M->>F: set_mode(OFFBOARD), если нужно
    F-->>M: system_state: OFFBOARD
    M->>M: Первый update_setpoint и запуск таймера
    loop С частотой update_rate_hz
        M->>F: position/velocity setpoint
    end
```

Если backend или FCU отклоняет переход в `OFFBOARD`, режим вызывает `activate_mode` с пустым именем и освобождает lease.

## Отзыв и завершение

При потере lease или разрыве bond `mode_base` останавливает таймер `update_setpoint(dt_s)` и вызывает `on_deactivate()`. Деструктор режима тоже останавливает отправку setpoint.

Failsafe отзывает lease на любом этапе. До первого состояния armed + `OFFBOARD` промежуточные состояния входа в `OFFBOARD` не влияют на lease. После подтверждения bridge отзывает lease при смене режима FCU или disarm.

```{mermaid}
sequenceDiagram
    participant N as Потребитель или режим
    participant F as FCU
    participant B as /fcu_bridge
    participant M as mode_base

    alt Явное завершение
        N->>B: ActivateMode с пустым именем
    else Переключение режима
        N->>B: ActivateMode другого режима
    else Takeover, failsafe или disarm
        F-->>B: system_state изменился
    else Узел режима завершил работу
        M--xB: bond разорван
    end
    B-->>M: mode_state с mode_id = 0 или другим id
    M->>M: Остановка таймера и on_deactivate()
```

Метод `completed()` завершает режим и асинхронно освобождает lease запросом с пустым именем. При переключении между companion-режимами FCU остаётся в `OFFBOARD`.

Если backend не поддерживает нативную регистрацию режимов, bridge хранит их в своём реестре. Выполнение таких режимов происходит через `OFFBOARD`.
