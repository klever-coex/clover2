# Nodes

In the ROS 2 ecosystem, a node is a discrete software component designed to perform a specific task — such as capturing a camera feed, processing sensor data, or detecting ArUco markers.

The system's logic is intentionally decoupled into these separate nodes to streamline development, testing, and maintenance. Because nodes run independently, the failure or shutdown of a single node does not necessarily result in a total system crash, providing much-needed fault tolerance.

Furthermore, ROS 2 is language-agnostic: nodes can be written in various programming languages, provided a ROS 2 client library (RCL) exists for that language.

````{list-table}
:header-rows: 1
:widths: 30 90 90

* - Language
  - Library
  - Support

* - `Python`
  - [`rclpy`](https://github.com/ros2/rclpy/)
  - Full

* - `C++`
  - [`rclcpp`](https://github.com/ros2/rclcpp/)
  - Full

* - `C`
  - [`rclc`](https://github.com/ros2/rclc/)
  - Full

* - `Rust`
  - [`ros2_rust`](https://github.com/ros2-rust/ros2_rust/)
  - Experimental

````

## Simple Examples

To illustrate the concept, let’s look at two minimal examples. Both nodes perform the exact same action: they publish an incrementing string to a `/topic` every 500 ms using a timer.

When implementing nodes, it is critical to understand the role of `rclpy.spin(node)` in Python and `rclcpp::spin(node)` in C++. These calls hand control over to the executor, the engine that processes the node's events, such as timer callbacks, incoming messages, and service requests.
Without the `spin()` function, the program would initialize the node and the timer but would never actually execute the callback functions. For instance, when you define a `timer_callback`, it is the `spin()` loop that listens for the timer trigger and subsequently executes that function.

````{tab-set-code}
```python
import rclpy
from std_msgs.msg import String

def main(args=None):
    counter = 0

    rclpy.init(args=args)

    node = rclpy.create_node('minimal_publisher')    
    publisher = node.create_publisher(String, 'topic', 10)

    def timer_callback():
        nonlocal counter
        msg = String()
        msg.data = f'Hello World: {counter}'
        publisher.publish(msg)
        node.get_logger().info(f'Publishing: "{msg.data}"')
        counter += 1

    timer = node.create_timer(0.5, timer_callback)
    rclpy.spin(node)

    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
```

```c++
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <chrono>

int main(int argc, char * argv[]) {
    int counter = 0;

    rclcpp::init(argc, argv);

    auto node = std::make_shared<rclcpp::Node>("minimal_publisher");
    auto publisher = node->create_publisher<std_msgs::msg::String>("topic", 10);

    auto timer_callback = [&]() {
        auto msg = std_msgs::msg::String();
        msg.data = "Hello World: " + std::to_string(counter);
        publisher->publish(msg);
        RCLCPP_INFO(node->get_logger(), "Publishing: '%s'", msg.data.c_str());
        counter++;
    };

    auto timer = node->create_wall_timer(
        std::chrono::milliseconds(500),
        timer_callback
    );

    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
```
````

`spin()` does not inherently imply single-threaded processing. While the default executors in `rclpy.spin()` and `rclcpp::spin()` process callbacks sequentially in a single thread, ROS 2 also supports multi-threaded executors. These allow compatible callback functions to be executed in parallel.

Avoid performing long-running or blocking operations (such as `time.sleep(...)`) inside a `callback` function. In a single-threaded executor, a blocking call will halt the entire thread, preventing all other callbacks in that node from executing until the operation completes.

## Advanced Examples

For scalable applications, the industry standard is to encapsulate a node's logic within a class. In this architecture, the node's settings, publishers, subscribers, and timers are stored as class properties.

````{tab-set-code}
```python
import rclpy
from rclpy.node import Node
from std_msgs.msg import String

class MinimalPublisher(Node):
    def __init__(self):
        super().__init__('minimal_publisher')
        self.publisher_ = self.create_publisher(String, 'topic', 10)
        timer_period = 0.5
        self.timer = self.create_timer(timer_period, self.timer_callback)
        self.counter = 0

    def timer_callback(self):
        msg = String()
        msg.data = f'Hello World: {self.counter}'
        self.publisher_.publish(msg)
        self.get_logger().info(f'Publishing: "{msg.data}"')
        self.counter += 1

def main(args=None):
    rclpy.init(args=args)
    node = MinimalPublisher()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
```

```c++
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/string.hpp>

#include <chrono>

class MinimalPublisher : public rclcpp::Node
{
public:
    MinimalPublisher()
    : Node("minimal_publisher"), counter_(0)
    {
        publisher_ = this->create_publisher<std_msgs::msg::String>("topic", 10);
        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(500),
            std::bind(&MinimalPublisher::timer_callback, this)
        );
    }

private:
    void timer_callback()
    {
        auto msg = std_msgs::msg::String();
        msg.data = "Hello World: " + std::to_string(counter_);
        publisher_->publish(msg);
        RCLCPP_INFO(this->get_logger(), "Publishing: '%s'", msg.data.c_str());
        counter_++;
    }

    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr publisher_;
    rclcpp::TimerBase::SharedPtr timer_;
    int counter_;
};

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<MinimalPublisher>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
```
````

While this approach may seem more complex initially, it becomes indispensable as a project grows. Additionally, in C++, this pattern enables the creation of [composable nodes] (https://docs.ros.org/en/jazzy/Concepts/Intermediate/About-Composition.html).
Composable nodes can be loaded into a single process, which drastically reduces the computational overhead of passing large data packets (such as high-resolution images) between separate processes. While `ROS 1` utilized a similar mechanism known as Nodelets, `ROS 2` uses a more robust composition framework to allow compatible nodes to share a single process space.

## Name and Namespace

In the examples above, the string minimal_publisher was passed to the constructor each time; this defines the ***node name***. The node name is the unique identifier used to locate the node within the ROS 2 graph.

The second key attribute is the ***namespace***. Namespaces allow you to organize nodes and resources into a logical hierarchy. A node's fully qualified name is a combination of its namespace and its node name.

If no namespace is specified, the node is created in the root namespace`/minimal_publisher`. Within a single namespace, no two nodes can share the same full name.

You can view all currently active nodes using the `ros2 node list` command:

```bash
$ ros2 node list
/minimal_publisher
```

Names and namespaces serve distinct purposes:

- **Name** identifies a specific instance of a task. For example, in a `Clover2` system, the `camera_node` driver might be launched twice: once as `main_camera` and once as `front_camera`. While both nodes run the same executable code, they exist as two distinct entities within the ROS 2 graph thanks to their unique names.
- **Namespaces** allow you to group related nodes and prevent naming conflicts. Crucially, relative names for topics and services automatically incorporate the namespace. For example, a camera node running in the `/front` namespace will publish images to `/front/image_raw`, whereas the exact same node running in the `/main namespace` will publish to `/main/image_raw`.

ROS 2 names and namespaces are restricted to Latin letters, digits, and underscores.

The names and namespaces defined within your source code are not set in stone. They can be overridden during the launch process without requiring any changes to the underlying code. 

## How to Launch Nodes

In ROS 2, a node is an executable file contained within a specific package. To manage and launch these executables, you can use the following commands:

| Command | Description |
| --- | --- |
| `ros2 pkg executables <пакет>` | Lists all executable files available within a specific package |
| `ros2 run <пакет> <исполняемый файл>` | Launches a single specific executable from a package |

To override a node's name or namespace at runtime, use the `--ros-args` flag along with remapping rules `(-r)`:

```bash
# Launch the camera driver under a new node name: front_camera
ros2 run camera_ros camera_node --ros-args -r __node:=front_camera

# Launch the same node within the /front namespace
ros2 run camera_ros camera_node --ros-args -r __node:=camera -r __ns:=/front
```

## Parameters

***Parameters*** serve as a node's configuration settings. Each parameter is defined by a name, a data type, and a value.

In your source code, a parameter must be explicitly declared. During declaration, you define both its name and its default value. Depending on your specific ROS 2 version and node configuration, attempting to pass a value for an undeclared parameter may be rejected by the system.

````{tab-set-code}
```python
node = rclpy.create_node('frequency_talker')

# Declaring parameters with names and default values
node.declare_parameter('frequency', 0.5)
node.declare_parameter('message', 'Hello World')

# Retrieving the current values
frequency = node.get_parameter('frequency').value
message = node.get_parameter('message').value
```

```c++
auto node = std::make_shared<rclcpp::Node>("frequency_talker");

// Declaring parameters with names and default values
node->declare_parameter<double>("frequency", 0.5);
node->declare_parameter<std::string>("message", "Hello World");

// Retrieving the current values
double frequency = node->get_parameter("frequency").as_double();
std::string message = node->get_parameter("message").as_string();
```
````

```{note}
The parameter type is strictly determined at the time of declaration. If you attempt to assign a value of an incompatible type, ROS 2 will reject the update.
```

If a parameter is not explicitly provided during startup, the system will fall back to the predefined default value. There are several ways to provide parameter values:

* **Via Command Line.** You can pass assignments directly to the `ros2 run` command using the `-p` flag:

```bash
ros2 run my_package frequency_talker --ros-args \
    -p frequency:=2.0 \
    -p message:="Hello, World"
```

For complex types like lists, use square brackets `-p image_size:="[256, 384]"`.

* **To a Running Node.** Parameters can be read or modified dynamically while the system is active using the `ros2 param get` and `ros2 param set` commands. For details, see [ROS 2 Commands](Commands).

* **Via Parameter Files.** For complex configurations involving many parameters, it is best practice to use a YAML file. This allows you to manage all settings in a single, organized document.

```yaml
/**:
  ros__parameters:
    use_intra_process_comms: true

/**/aruco_tracker:
  ros__parameters:
    tracking: "base_link"

/**/led_strip:
  ros__parameters:
    brightness_scale: 0.5
    led_count: 80
```

In a YAML parameter file, the top-level key acts as a pattern used to select target nodes: `/ matches` all nodes in any namespace, `//aruco_tracker` matches any node named `aruco_tracker`, regardless of its namespace.

This pattern-based approach allows a single file to configure multiple nodes simultaneously. Within the file, the `ros__parameters` key is a mandatory structural element; all specific parameter definitions must be nested under it.

To apply these settings, pass the file to your node at launch using the `--params-file flag`.

```bash
ros2 run my_package frequency_talker --ros-args --params-file my_params.yaml
```

## Lifecycle Nodes

Beyond standard nodes, ROS 2 supports [`Lifecycle Nodes`](https://design.ros2.org/articles/node_lifecycle.html), which provide explicit control over a node's internal state and the sequence of its startup and shutdown processes.
While a regular node begins its execution immediately upon launch, a Lifecycle node operates as a Finite State Machine (FSM).

```{mermaid}
stateDiagram-v2
    [*] --> Unconfigured: create

    Unconfigured --> Configuring: configure
    Configuring --> Inactive: on_configure() == SUCCESS
    Configuring --> Unconfigured: on_configure() == FAILURE
    Configuring --> ErrorProcessing: on_configure() == ERROR

    Inactive --> Activating: activate
    Activating --> Active: on_activate() == SUCCESS
    Activating --> ErrorProcessing: on_activate() == ERROR

    Active --> Deactivating: deactivate
    Deactivating --> Inactive: on_deactivate() == SUCCESS
    Deactivating --> ErrorProcessing: on_deactivate() == ERROR

    Active --> ErrorProcessing: Unhandled error

    Inactive --> CleaningUp: cleanup
    CleaningUp --> Unconfigured: on_cleanup() == SUCCESS
    CleaningUp --> ErrorProcessing: on_cleanup() == ERROR

    Unconfigured --> ShuttingDown: shutdown
    Inactive --> ShuttingDown: shutdown
    Active --> ShuttingDown: shutdown
    ShuttingDown --> Finalized: on_shutdown() == SUCCESS

    ErrorProcessing --> Unconfigured: on_error() == SUCCESS
    ErrorProcessing --> Finalized: on_error() == FAILURE / ERROR

    Finalized --> [*]: destroy
```

This architecture allows an external management component to precisely orchestrate when a node is configured, activated, deactivated, or shut down, ensuring a more predictable and robust system behavior.

A Lifecycle node moves through several primary states:

- `Unconfigured`. The node has been instantiated but has not yet had its parameters or resources initialized.
- `Inactive`. The node has been successfully configured, but its main execution logic is not yet running.
- `Active`. The node is fully operational and performing its primary intended tasks.
- `Finalized`. The node has completed its lifecycle and is prepared to be destroyed.

Transitions between these states are managed through standardized interfaces. Specifically, the Lifecycle node provides a service named `/<node_name>/change_state`. By calling this service, an external controller can request the node to transition from one state to another, allowing for controlled, step-by-step system initialization and teardown.
