# clover2_common

The `clover2_common` package provides essential classes and helper utilities shared across various modules. 
It includes specialized wrappers for standard ROS 2 classes, interfaces for managing diagnostics and parameter monitoring, and several high-level utility tools.

## Wrapper classes for standard C++ nodes

Wrapper classes extend the standard ROS 2 C++ node classes. 
These wrappers introduce additional methods that unify node management and allow for the seamless reuse of common functionality throughout the project.

Both `node` and `lifecycle_node` classes offer an identical set of extended methods.

A key feature is the `declare_and_watch_parameter<ParameterT>(name, default_value, cb, ...)` method. 
This allows you to declare a node parameter and simultaneously register a callback function that triggers automatically whenever the parameter's value changes.

In the example below, we declare a `frame_id` parameter with a default value of `base_link`. 

```cpp
    declare_and_watch_parameter<std::string>(
        "frame_id", "base_link",
        [this](const rclcpp::Parameter& p) { m_frame_id = p.as_string(); },
        "Tracking target");
```

When the parameter is updated, the callback updates the internal`m_frame_id` variable.

Full API Reference for the `node` class:

```{eval-rst}
.. doxygenclass:: clover2_common::node
    :members:
```

<br>

### `clover2_common::node_context`

The `node_context` class consolidates various node interfaces into a single object. 
It provides a unified way to interact with both standard ROS 2 interfaces and project-specific features, such as:
* `NodeDiagnosticsInterface` — for managing diagnostic tasks.
* `NodeParametersWatcherInterface` — for monitoring parameter changes.

Essentially `node_context` functions similarly to `rclcpp::node_interfaces::NodeInterfaces`, but is optimized for the `clover2` ecosystem.

Full API Reference for `node_context`:

```{eval-rst}
.. doxygenclass:: clover2_common::node_context
    :members:
```

<br>

### `clover2_common::node_runtime`

The `node_runtime` class is designed for complex applications that require parallel execution. 
It runs a `clover2_common::node` in a dedicated thread with its own `SingleThreadedExecutor`. 
This is particularly useful for programs that manage multiple execution loops, such as those utilizing `Boost.Asio` or `cpptui`.

Full API Reference for `node_runtime`:

```{eval-rst}
.. doxygenclass:: clover2_common::node_runtime
    :members:
```

## Custom node interfaces

The `clover2_common` introduces custom interfaces to simplify two critical tasks: managing node diagnostics and monitoring parameter updates.

### NodeDiagnosticsInterface / NodeDiagnostics

The `NodeDiagnosticsInterface` provides a structured way to access and manage a node's diagnostic tasks. 
Built upon the `diagnostic_updater` library, it enables typed access to various diagnostic components.

For instance, you can add a `lifecycle_state_task` and define a `getter` function that automatically reports the node's current state:

```cpp
get_node_diagnostics_interface()->add<diagnostics::lifecycle_state_task>();
...
get_node_diagnostics_interface()
    ->get<diagnostics::lifecycle_state_task>()
    .set_state_getter([this]() { return get_current_state(); });
```

Interface Specification:

```{eval-rst}
.. doxygeninterface:: clover2_common::node_interfaces::NodeDiagnosticsInterface
    :members:
```

<br>

### NodeParametersWatcherInterface / NodeParametersWatcher

The `NodeParametersWatcherInterface` is a specialized tool for monitoring changes of node parameters. 
Once a parameter's value is modified, a callback is registered.

Interface Specification:

```{eval-rst}
.. doxygeninterface:: clover2_common::node_interfaces::NodeParametersWatcherInterface
    :members:
```

<br>

Both `NodeDiagnosticsInterface` and `NodeParametersWatcherInterface` are pre-implemented within the `clover2_common` package and can be accessed directly through your node object.
