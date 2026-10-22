# Simulator

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Simulator/Installation
Simulator/Usage
```

```{figure} @assets@/common/simulator/sim-screenshot.webp
:alt: Gazebo
:width: 90%
:align: center

Gazebo simulator
```
<br>

## What Is a Simulator?

**Simulator** is a specialized software environment that creates a high-fidelity virtual replica of the real world. It allows you to run a digital model of a quadcopter to observe its flight dynamics, movement patterns, and reactions to surrounding objects. Beyond movement, the simulator emulates sensor operations and transmits data directly to your control software. The value of a simulator lies in its accuracy: the more precisely it reproduces real-world physics, the more reliable your algorithms will be when transitioned to hardware.

Modern simulators can model:

* **Physics:** gravity, inertia, air resistance, and collisions. These parameters ensure the quadcopter’s movement mirrors real-world flight behavior.
* **Sensors:** cameras, LiDAR, GPS, gyroscopes, and rangefinders. The simulator generates data that closely matches real-world readings, including realistic sensor noise and measurement errors.
* **Environment:** walls, floors, obstacles, lighting, and textures. This allows the virtual quadcopter to navigate complex spaces and interact with digital objects.

By using a simulator, you can test code and algorithms without the need for physical hardware. This workflow enables you to identify and fix errors in a safe environment, significantly reducing the risk of damaging expensive equipment during the development process.

## How a Simulator Works

A quadcopter simulator typically consists of three core components working in synchronization:

1. **Physics engine** calculates object movement and interactions. By factoring in the quadcopter's mass, motor thrust, and gravity, it determines how the vehicle’s position and orientation change over time.
2. **Graphics engine** renders the virtual world on your screen, including the environment, obstacles, and the quadcopter itself. It is also responsible for generating the visual feed captured by the virtual camera.
3. **Sensor models** emulate the behavior of physical hardware. For instance, a virtual camera transmits an image stream, a virtual GPS provides coordinates, and a virtual IMU reports acceleration and angular velocity.

These three parts operate as a single ecosystem. The physics engine calculates movement; the graphics engine visualizes that movement within the environment; and the sensor models collect data from that movement to transmit to your control program. Consequently, your software receives the exact same type of information it would encounter on a physical quadcopter.

## Simulator and ROS 2

ROS 2 allows your control program to exchange data with the quadcopter and its sensors via topics, services, and messages. In the simulator, you can configure these ROS 2 data exchanges so that your software interacts with the virtual quadcopter through the identical interfaces used by real hardware. This seamless integration allows you to develop and validate algorithms in a virtual space before deploying them to a physical device.

```{caution}
A simulator cannot fully replicate every unpredictable condition of a real-world flight.
Always verify program compatibility, configuration settings, and algorithm behavior on the physical device before attempting a real flight.
```

## Try It Yourself

To get started, select the appropriate {doc}`installation <Simulator/Installation>` option for your operating system:: 
* Dev Container — for Linux.
* VMware — fow Windows.
* UTM — for macOS.
  
Once installation is complete, refer to the {doc}`Usage <Simulator/Usage>` section to launch the simulator and begin interacting with your virtual quadcopter.
