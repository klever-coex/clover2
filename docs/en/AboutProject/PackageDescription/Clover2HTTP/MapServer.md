# `map_server`

`map_server` is a plugin within the `clover2_ui/clover2_http_plugins` package that provides a REST CRUD (Create, Read, Update, Delete) interface for managing ArUco map markers. 
This plugin allows users and applications to retrieve map data, add new markers, update existing marker parameters, or remove markers.

All modifications are forwarded to the map node using the `clover2_pose_msgs/srv/ModifyMap` service. For read operations, the plugin utilizes a cached snapshot previously obtained by the client from the map node.

## Routes

The plugin exposes the following HTTP routes for map management:

| Method | Route | Description |
| --- | --- | --- |
| GET | `/api/map` | Retrieves the complete map data, including metadata (name, frame, dictionary) and the full list of markers |
| GET | `/api/map/marker/-/{id}` | Retrieves the details of a marker by its  `id` |
| POST | `/api/map/marker` | Creates and adds a new marker to the map |
| PUT | `/api/map/marker/-/{id}` | Updates the parameters of an existing marker. The `id` in the URL path must match the `id` in the request body |
| DELETE | `/api/map/marker/-/{id}` | Removes a marker from the map by its `id` |

## Parameters

The plugin is configured using the following parameter:

| Parameter | Default | Description |
| --- | --- | --- |
| `map_server.map_node` | `map_server` | Map node that the plugin communicates with |

## Modifying the Map

When performing an add or modify operation, include a `marker_info` object in the HTTP request body.
The `marker_info` object contains the following fields:
*	`id` — unique identifier for the marker.
*	`type` — specific type of marker.
*	`size` — physical size of the marker.
*	`marker_frame_id` — frame identifier for the marker.
*	`pose` — position and orientation of the marker.
  
Upon receiving a request, the server responds with a `modify_result` object:

```json
{"success": true, "error_message": ""}
```

* `success` — a boolean indicating if the operation was successful.
*  `error_message` — a string containing description of the error if success is false.

The plugin performs client-side validation before forwarding requests to the map node. An HTTP `400` (Bad Request) error will be returned if:

*	The identifier is a negative number.
*	The marker size is not a positive value.
*	An unsupported marker type is specified.

## Capability and Availability

The plugin defines its primary capability as `map`. This capability is included in the plugin's manifest (`GET /api/manifest`) only after a successful connection to the map node has been established. If the map node is unavailable, the plugin will periodically attempt to reconnect.

* If the map node service returns an error, the server returns a `modify_result{false}`.
* If the map node service fails to respond within 5 seconds, the server returns an HTTP `504` (Gateway Timeout).
