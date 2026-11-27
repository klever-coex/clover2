# `ros_support`

`ros_support` is a universal plugin within the `clover2_http` package. It provides a comprehensive REST API allowing you to retrieve lists of nodes and topics, examine publishers, subscribers, and services for individual nodes, and manage Lifecycle nodes. A WebSocket interface is provided to stream topic messages directly to your client.

To ensure maximum flexibility, `ros_support` leverages the `rosidl_typesupport_introspection_cpp` mechanism for message serialization. This allows the plugin to remain agnostic of specific message definitions; it can dynamically handle and serialize various message types without requiring hardcoded structures in the plugin itself.

## Routes

The plugin exposes the following HTTP endpoints and WebSocket routes for interacting with the ROS 2 ecosystem:

| Method | Path | Description |
| --- | --- | --- |
| GET | `/api/topics` | Returns a list of all active topics in the graph `{name, type}` |
| GET | `/api/nodes` | Returns a list of all active node names. |
| GET | `/api/node/info/-/{node...}` | Retrieves detailed node information: name, namespace, lifecycle capability, and current state |
| GET | `/api/node/publishers/-/{node...}` | ПLists the node's publishers, including the associated topics and QoS profiles |
| GET | `/api/node/subscribes/-/{node...}` | Lists the node's subscribers, including the associated topics and QoS profiles |
| GET | `/api/node/servers/-/{node...}` | Returns a list of services provided by the node |
| GET | `/api/node/clients/-/{node...}` | Returns a list of service clients owned by the node |
| GET | `/api/node/lifecycle/available_transitions/-/{node...}` | Lists all valid state transitions available from the node's current lifecycle state |
| POST | `/api/node/lifecycle/transition/-/{node...}` | Triggers a state transition for a Lifecycle node |
| WS | `/ws/topic/json/-/{topic...}` | Establishes a WebSocket connection to stream topic messages in JSON format |

Supported capabilities: `nodes`, `topics`, `services`.

## Parameters

| Parameter | Default | Description |
| --- | --- | --- |
| `ros_support.topics_rate_limit_bps` | `100000.0` | Sets the maximum aggregate data transfer rate for all topics (in bytes per second) |

## Managing Lifecycle Nodes

ROS 2 utilizes a standardized lifecycle management mechanism, where nodes transition through specific states via managed services. The `ros_support plugin` simplifies this process by allowing you to query valid transitions and execute them directly through the REST API.

### How to Retrieve Available Transitions

To see which state transitions are currently permitted for a specific node, send a GET request to: `GET /api/node/lifecycle/available_transitions/-/{node...}`.

The server will respond with a list of available transitions relative to the node's current state. Each transition object includes:

* `transition` — detailed information regarding the transition action.
* `start_state` — node's current state before the transition.
* `goal_state` — resulting state after a successful transition.

Example response:

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

### How to Perform a Transition

To trigger a state change in a Lifecycle node, send a POST request to: `POST /api/node/lifecycle/transition/-/{node...}`.

The request body must contain a JSON object with a `label` field specifying the name of the transition you wish to execute.

Example: To trigger the `configure` transition for a node named `map_server`, you can use the following curl command:

```bash
curl -X POST http://localhost:8080/api/node/lifecycle/transition/-/map_server \
    -H 'Content-Type: application/json' \
    -d '{"label": "configure"}'
```

Possible HTTP response codes:

- `200` — transition was executed successfully.
- `409` — node rejected the transition (e.g., attempting to `activate` while the node is still in the `unconfigured` state).
- `502` — failed to call the node's internal service (e.g., the node is unreachable).

## Message Streaming

`ros_support` streams ROS 2 messages as individual text frames over a WebSocket connection. The plugin performs the following serialization:

* JSON field names match the field names defined in the original ROS 2 `.msg` file.
* Nested messages are converted into nested JSON objects.
* : ROS 2 arrays are converted into standard JSON arrays.
  
Example: A message published on the `/rosout type rcl_interfaces/msg/Log` will be serialized as follows:

```json
{"file":"./src/topics/talker.cpp","function":"operator()","level":20,"line":47,"msg":"Publishing: 'Hello, 0'","name":"talker","stamp":{"nanosec":521751302,"sec":1756200000}}
```

If the server cannot establish the WebSocket connection, it will return an HTTP error response containing a JSON object with the error details:`{"error": "<message>"}`. 

If you receive any status code other than `200`, please inspect the `error` field for troubleshooting.

Once a connection is established, errors or reasons for termination are communicated via standard WebSocket close codes:

- `1008` — requested topic does not exist in the ROS 2 graph.
- `1011` — unexpected error occurred on the server.
- `1001` — server closed the connection due to 60 seconds of inactivity.

```{important}
To prevent the server from closing the connection due to inactivity, the client must periodically send text frames to the server.
While the server ignores the content of these frames, receiving them resets the 60-second inactivity timer.
```

## Stream Limitations

To ensure efficient message delivery, the plugin creates generic subscriptions using the  `best_effort` Reliability QoS and a queue depth of `10`. Under high network load or heavy message frequency, some messages may be dropped.

Total data throughput is governed by the `ros_support.topics_rate_limit_bps` parameter. This limit is applied globally across all active topic streams.
