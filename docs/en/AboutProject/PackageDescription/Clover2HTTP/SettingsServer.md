# `settings_server`

`settings_server` is a plugin within the `clover2_ui/clover2_http_plugins` package that provides a REST API for viewing and modifying the launch settings of the `clover2` package. The plugin uses the `clover2_ui::api::settings::config_field` class to work with the configuration. The TUI settings editor, which is launched with the `clover2_ui settings` command, is based on the same class.

The configuration consists of two parts:
*	**YAML schema** describes the structure of the settings: field names and types, default values, descriptions, and allowed values.
*	**YAML values file** contains user values that override the default values from the schema.

## Routes

The plugin provides the following HTTP routes:

| Method | Path | Description |
| --- | --- | --- |
| GET | `/api/settings/schema` | Get the YAML settings schema with the current field values (`value` -> otherwise `default`) |
| PUT | `/api/settings/values` | Submit a complete set of settings values for saving |

## Parameters

The plugin uses the following parameters:

| Parameter | Default | Description |
| --- | --- | --- |
| `settings_server.schema_path` | *(empty)* | Path to the YAML settings schema file, for example `klever5.yaml` from the `clover2_bringup` package. If the path is not specified, the plugin is disabled |
| `settings_server.values_path` | *(empty)* | Path to the YAML values file. The same configuration file that is passed to the launcher and applied in the TUI editor is used |

## How to Read the Schema

To get the structure of the settings and their current values, send a GET request: `GET /api/settings/schema`.
The server returns a JSON object with the settings schema.
Each schema field may contain the following parameters:
*	`name` — field name.
*	`type` — data type.
*	`description` — description of the setting.
* `default` — default value.
* `enum` — list of allowed values.
* `children` — nested fields for objects of type `object`.
* `value` — current value of the field, if it is set in the values file.

Example response:

```json
{"valid": true, "root": {"children": [{"name": "fcu_bridge", "type": "object",
  "description": "Flight controller unit bridge", "children": [
    {"name": "fcu_conn", "type": "str", "description": "FCU connection type",
     "default": "uart", "enum": ["usb", "uart", "tcp", "udp"], "value": "tcp"}]}]}}
```

## How to Save Values

To modify the settings, send a PUT request: `PUT /api/settings/values`. In the request body, pass a `values` JSON object with the complete tree of current values. The data format corresponds to the configuration structure used by the TUI editor.

Example request:

```json
{"values": {"fcu_bridge": {"fcu_conn": "tcp"}, "main_camera": {"enable": true}}}
```

If the request contains an unknown key, a value of an unsuitable type, or a value that is not present in the `enum` list, the server returns HTTP code `400` (Bad Request).

The plugin's capability is `settings`. It is included in the manifest (`GET /api/manifest`) only after the YAML schema has been successfully loaded. If the schema has not been loaded, the plugin's routes return HTTP code `503` (Service Unavailable).
