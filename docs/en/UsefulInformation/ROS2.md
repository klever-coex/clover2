# ROS 2

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

ROS2/Nodes
ROS2/Commands
```

```{figure} @assets@/common/programming/ros2/ros2-logo.webp
:alt: Docker
:width: 20%
```
<br>

## Introduction

Developing software for robotics introduces a unique layer of complexity. Unlike traditional software, a robot must simultaneously manage complex algorithms and physical hardware.

A robotic program manages not just logic, but a real-world devices: cameras, distance sensors, electric motors, flight controllers, GPS, and more. These components constantly exchange data that the program must receive, process, and interpret in real-time to make informed decisions.

* **Multitasking Demands.** A single program must handle diverse responsibilities. For instance, one module stabilizes a quadcopter in mid-air, another processes camera feeds, a third identifies obstacles, and a fourth calculates the next flight path.
* **Intricate Data Exchange.** Communication must be seamless. A camera must transmit images to a recognition algorithm; that algorithm must then alert the control system to an obstacle; and the control system must instantly adjust the flight trajectory.
* **Hardware Integration.** A quadcopter integrates numerous sensors and actuators—including altitude sensors, GPS, IMUs (Inertial Measurement Units), rangefinders, and motors. All these devices must operate in perfect synchronization.
* **Scalability Issues.** Projects naturally evolve. A quadcopter that begins with simple takeoff and landing capabilities may eventually require position holding, waypoint navigation, and object recognition. Without a proper framework, the codebase quickly becomes too large and unwieldy to maintain.

To address these challenges, engineers use a specialized suite of development tools: ***Robot Operating System 2***, commonly known as **ROS 2**.
Despite its name, ROS 2 is not an operating system in the traditional sense. Rather, it is a powerful collection of software libraries, tools, and interfaces designed to help developers build individual robotic components and organize their interactions efficiently.

The philosophy behind ROS 2 is straightforward: instead of building one massive, monolithic program, the robot is composed of many small, specialized programs called **nodes**. Each node is responsible for a single, specific task.

For example, a quadcopter might utilize the following nodes:
* **Flight Controller Node** communicates with the autopilot to receive telemetry (altitude, speed, position) and issues commands to take off, adjust altitude, or land.
* **Camera Node** captures images from the onboard camera and publishes them for other nodes to use.
* **ArUco Marker Node** analyzes camera feeds to detect ArUco markers and determines their position relative to the quadcopter—a critical function for precision landing.
* **Navigation Node** consolidates data from various sources (such as GPS or ArUco positions) to calculate the optimal flight path.
* **Operator Command Node** relays human inputs from a keyboard, remote control, or ground station to the rest of the system.

ROS 2 provides standardized mechanisms for these nodes to exchange information. The most common method is through `Topics` — named channels used for transmitting messages.

In a quadcopter workflow, the data exchange might look like this:
* The Camera Node publishes images to the `/camera/image`topic.
* The ArUco Marker Node subscribes to the `/camera/image` to receive and process those images.
* If a marker is detected, the node publishes its coordinates to the `/aruco/pose`topic.
* The Navigation Node subscribes to `/aruco/pose` and calculates the necessary adjustments to approach the marker for a precise landing.

The primary strength of this architecture is **decoupling**. Nodes do not depend directly on one another; they simply send and receive messages. ROS 2 handles the complex task of ensuring that data reaches the correct destination.
This modular approach makes the entire system significantly easier to modify, debug, and scale, allowing developers to add new features without rewriting the entire robot's intelligence.

Beyond communication, ROS 2 provides a vast ecosystem of ready-made tools designed to streamline robotic development:
* **Coordinate Transformations**. In robotics, spatial awareness is critical. You must constantly track where the robot is, where the camera is positioned, and where an object is located in space. ROS 2 includes sophisticated tools for defining coordinate frames and performing transformations between them (e.g. translating data from the camera's perspective to the robot, and finally to a global map).
* **Data Visualization**. ROS 2 allows you to monitor the robot’s position, sensor streams, movement trajectories, and LiDAR point clouds as they happen. This visual feedback is essential for understanding system behavior during development.
* **Simulation Environments**. ROS 2 integrates with powerful simulators, allowing you to test navigation, control logic, and data processing in a safe, virtual environment. 
* **Hardware Drivers and Specialized Packages**. ROS 2 offers a massive library of pre-built drivers and packages for popular cameras, LiDARs, GPS modules, and other essential hardware. This allows you to integrate sophisticated sensors into your system almost immediately.

At its core, ROS 2 serves as the bridge that integrates disparate software components into a single, cohesive robotic system. By allowing you to break down complex challenges into small, independent, and manageable modules, it makes development, testing, and continuous improvement far more efficient.
Whether you are building a simple prototype or a sophisticated autonomous machine, ROS 2 provides the foundation necessary to turn complex ideas into functional reality.
