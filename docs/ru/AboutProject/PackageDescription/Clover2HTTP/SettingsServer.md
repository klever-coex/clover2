# `settings_server`

Плагин пакета `clover2_ui/clover2_http_plugins`. Предоставляе доступ к конфигурации launch запуска, через `clover2_ui::api::settings::config_field` — того же класса, на котором построен TUI-редактор `clover2_ui settings`. Схема и значения хранятся в YAML: схема описывает дерево полей (`type`: `str`/`bool`/`int`/`float`/`object`, `default`, `description`, `enum`, вложенные `fields`), значения — переопределения поверх дефолтов.

## Маршруты

| Метод | Путь | Описание |
| --- | --- | --- |
| GET | `/api/settings/schema` | Дерево схемы; на каждом листе — текущее значение (`value` -> иначе `default`) |
| PUT | `/api/settings/values` | Заменить значения |

## Параметры

| Параметр | По умолчанию | Описание |
| --- | --- | --- |
| `settings_server.schema_path` | *(пусто)* | Путь к YAML-схеме, например `klever5.yaml` из `clover2_bringup`; пусто — плагин отключён |
| `settings_server.values_path` | *(пусто)* | Путь к файлу значений (тот же конфиг, что подаётся в лаунчер и используется для TUI) |

## Чтение схемы

```json
{"valid": true, "root": {"children": [{"name": "fcu_bridge", "type": "object",
  "description": "Flight controller unit bridge", "children": [
    {"name": "fcu_conn", "type": "str", "description": "FCU connection type",
     "default": "uart", "enum": ["usb", "uart", "tcp", "udp"], "value": "tcp"}]}]}}
```

## Сохранение значений

Тело запроса — полное дерево текущих значений (формат совпадает с выводом TUI):

```json
{"values": {"fcu_bridge": {"fcu_conn": "tcp"}, "main_camera": {"enable": true}}}
```

- неизвестный ключ, несоответствие типа или значение вне `enum` — ошибка `400`;

Capability: `settings`. Присутствует в манифесте только если схема успешно загружена; иначе маршруты отвечают `503`.
