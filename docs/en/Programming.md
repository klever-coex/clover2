# Programming

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

Programming/ROS2
```

The `clover2` framework is built upon `ROS 2`, the industry-standard open-source platform for robotic development. This integration provides developers with a powerful, scalable environment to implement custom autonomous flight logic using any language supported by the `ROS 2` ecosystem.

The easiest way to get started is to use a dedicated `clover2` Python Wrapper, containing all the functionality needed to solve application-specific tasks. This wrapper sits atop the ROS 2 stack and interacts with the controller through standard ROS 2 topics and services, available to other system nodes.

For applications requiring maximum computational efficiency or highly specialized logic, developers can create custom nodes directly within the ROS 2 environment.
You can use C++ (rclcpp) for performance-critical tasks and Python (rclpy) for rapid iteration. Custom nodes exchange data seamlessly with all other system components via the standard ROS 2 middleware (topics, services, and actions).
