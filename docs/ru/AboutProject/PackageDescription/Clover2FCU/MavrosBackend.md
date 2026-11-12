# Бэкенд MAVROS

`clover2_fcu_mavros` реализует восемь capabilities через MAVROS. Имя протокола и pluginlib lookup name: `mavros`.

## Настройка адресов

Префикс топиков и сервисов задаётся в YAML-параметре `plugin_params` bridge.

```yaml
mavros:
  namespace: /mavros
```

Ключ `mavros.namespace` можно не задавать. По умолчанию используется `/mavros`. Остальные ключи `plugin_params` передаются провайдерам по имени capability через `context::params()`.

## Соответствие capabilities

В таблице `<prefix>` обозначает значение `mavros.namespace`.

| Capability | Топики и сервисы MAVROS | Ограничения и особенности |
| --- | --- | --- |
| `system_state` | Подписки `<prefix>/state` и `<prefix>/extended_state` | `failsafe` всегда равен `false`: текущий источник MAVROS не заполняет флаг |
| `odometry` | Подписка `<prefix>/local_position/odom` | Специфических ограничений нет |
| `battery` | Подписка `<prefix>/battery` | Заполняется только источник с `id == 0`; мультибатарейная телеметрия не различается |
| `commands` | `<prefix>/cmd/arming`, `<prefix>/set_mode`, `<prefix>/cmd/takeoff`, `<prefix>/cmd/land`; `rtl` вызывает `set_mode(AUTO.RTL)` | `arm()` игнорирует `run_preflight_checks`; для `takeoff()` нужна высота, heading можно не задавать; callback подтверждает приём команды |
| `parameters` | `<prefix>/param/pull`, `<prefix>/param/set`, подписка `<prefix>/param/event` | `get()` читает локальный кэш; `set()` ждёт ответа, поэтому его нельзя вызывать внутри callback FCU-группы |
| `estimator` | Издатели `<prefix>/odometry/out`, `<prefix>/global_position/set_gp_origin`, `<prefix>/optical_flow/raw/send` | `reset()` возвращает `unsupported`; `estimator_source` игнорируется; доступен один optical-flow publisher с `id == 0`, повторный вызов возвращает пустой handle |
| `setpoint_position` | Издатель `<prefix>/setpoint_raw/local` | Используется `FRAME_LOCAL_NED`; heading задаётся в радианах |
| `setpoint_velocity` | Издатель `<prefix>/setpoint_raw/local` | Используется `FRAME_LOCAL_NED`; yaw rate задаётся в рад/с |

```{mermaid}
flowchart LR
    subgraph IN["Входящие данные"]
        S["state + extended_state"] --> SS["system_state"]
        O["local_position/odom"] --> OD["odometry"]
        BAT["battery"] --> BC["battery"]
        PE["param/event"] --> PAR["parameters"]
    end

    subgraph OUT["Исходящие данные и запросы"]
        CMD["commands"] --> CS["cmd/arming · set_mode<br/>cmd/takeoff · cmd/land"]
        PAR --> PS["param/pull · param/set"]
        EST["estimator"] --> EO["odometry/out<br/>set_gp_origin<br/>optical_flow/raw/send"]
        SP["setpoint_position<br/>setpoint_velocity"] --> RAW["setpoint_raw/local"]
    end
```

Оба setpoint-провайдера отправляют `mavros_msgs/msg/PositionTarget`. Позиция и скорость задаются через `Eigen::Vector3d`.

При запуске параметры загружаются через `param/pull`. Затем кэш пополняется событиями `param/event`. Для записи используется `ParamSetV2`, отдельный сервис `param/get` не нужен.

MAVROS не поддерживает нативную регистрацию пользовательских режимов в данном API. `mode_base` регистрирует их в bridge и переводит FCU в `OFFBOARD` собственной командой.

MAVROS не передаёт `failsafe` напрямую. На lease режима такие события влияют косвенно, например через смену режима FCU или disarm.
