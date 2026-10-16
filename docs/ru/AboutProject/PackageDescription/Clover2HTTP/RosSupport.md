# `ros_support`

Универсальный плагин, входит в состав пакета `clover2_http`. Предоставляет REST API для графа ROS 2 (топики, ноды, их концы и сервисы) и односторонний веб-сокет стрим сообщений в формате JSON.

Сериализация сообщений построена на typesupport (`rosidl_typesupport_introspection_cpp`): плагин не зависит от конкретных типов сообщений и умеет стримить любой тип, доступный в среде(все setup.bash вызванные в терминале).

## Маршруты

| Метод | Путь | Описание |
| --- | --- | --- |
| GET | `/api/topics` | Все топики графа: `{name, type}` |
| GET | `/api/nodes` | Имена всех нод |
| GET | `/api/node/info/-/{node...}` | Информация о ноде: имя, неймспейс, признак lifecycle и текущее состояние |
| GET | `/api/node/publishers/-/{node...}` | Издатели ноды: топик и QoS-профиль |
| GET | `/api/node/subscribes/-/{node...}` | Подписчики ноды: топик и QoS-профиль |
| GET | `/api/node/servers/-/{node...}` | Сервисы ноды |
| GET | `/api/node/clients/-/{node...}` | Клиенты ноды |
| GET | `/api/node/lifecycle/available_transitions/-/{node...}` | Переходы, доступные из текущего состояния lifecycle-ноды |
| POST | `/api/node/lifecycle/transition/-/{node...}` | Выполнить переход lifecycle-ноды |
| WS | `/ws/topic/json/-/{topic...}` | Поток сообщений топика в JSON |

Capabilities: `nodes`, `topics`, `services`.

## Параметры

| Параметр | По умолчанию | Описание |
| --- | --- | --- |
| `ros_support.topics_rate_limit_bps` | `100000.0` | Ограничение скорости потока (байт/с) на одно подключение |

## Управление lifecycle-нодами

Для lifecycle-нод плагин позволяет управлять машиной состояний через REST.

`GET /api/node/lifecycle/available_transitions/-/{node...}` возвращает переходы, доступные из текущего состояния ноды. Каждый переход описан самим переходом и парой состояний (начальное и конечное):

```json
{
  "available_transitions": [
    {
      "transition": { "label": "configure" },
      "start_state": { "label": "unconfigured" },
      "goal_state": { "label": "inactive" }
    }
  ]
}
```

`POST /api/node/lifecycle/transition/-/{node...}` выполняет переход. Переход задается телом запроса — объектом с полем `label`, например `configure`, `activate`, `deactivate`, `cleanup`, `shutdown`:

```bash
curl -X POST http://localhost:8080/api/node/lifecycle/transition/-/map_server \
    -H 'Content-Type: application/json' \
    -d '{"label": "configure"}'
```

Возможные коды ответа:

- `200` — переход выполнен;
- `409` — нода отвергла переход (например, `activate` из состояния `unconfigured`);
- `502` — не удалось вызвать сервис ноды (нода недоступна).

## Стриминг сообщений

Одно ROS-сообщение — один текстовый фрейм. Поля сериализуются в JSON с именами как в определении `.msg`, вложенные сообщения — вложенные объекты, массивы — массивы. Пример фрейма топика `/rosout` (`rcl_interfaces/msg/Log`):

```json
{"file":"./src/topics/talker.cpp","function":"operator()","level":20,"line":47,"msg":"Publishing: 'Hello, 0'","name":"talker","stamp":{"nanosec":521751302,"sec":1756200000}}
```

Ошибки передаются фреймом `{"error": "<message>"}` и HTTP кодом ответа(на стороне клиента при получении ответа, если код != 200, надо проверить поле `error`). Коды закрытия веб-сокета:

- `1008` — топик отсутствует в графе на момент подключения;
- `1011` — внутренняя ошибка сервера;
- `1001` — idle-таймаут сервера (60 секунд без входящих фреймов): клиент должен периодически отправлять любые текстовые фреймы — сервер их игнорирует, но таймер сбрасывает.

```{important}
Клиент должен периодически отправлять любые данные, иначе сервер через 60 секунд закроет сокет автоматически.
```

## Ограничения потока

Подписка создаётся как generic с QoS `best_effort` и глубиной 10 — под нагрузкой сообщения могут пропускаться. Пропускная способность ограничивается по bps для каждого топика (параметр `ros_support.topics_rate_limit_bps`).
