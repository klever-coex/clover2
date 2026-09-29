# Coordinate Systems

A coordinate system is a mathematical framework used to specify the position of objects in space, consisting of an origin, axes, and axis orientation (`x`, `y` и `z`).

A frame is a named coordinate system tied to a specific object or area. 
Its position can be expressed relative to another frame.

```{note}
To manage frames on Klever quadcopters, we utilize the ROS package [tf2](https://index.ros.org/p/tf2/).
```

```{figure} @assets@/common/programming/frames/basic-frames.webp
:alt: Пример расположения фреймов в пространстве
:align: center

Example of frame arrangement in space
```

The primary frames in the `clover2` package follow the [REP 105](https://reps.openrobotics.org/rep-0105/) convention:

* `base_link` — Coordinate system attached to the quadcopter body
* `odom` — Coordinate system relative to the flight controller's initialization point
* `map` – Coordinate system relative to the [marker map](./ArucoMap) (this name can be customized in the configuration, though we recommend using the standard name for compatibility)

In this hierarchy, `base_link` moves and rotates alongside the quadcopter, while `odom` and `map` serve as fixed reference systems to determine the vehicle's position in space.

Additionally, a unique frame is generated for every marker in the map. By default, these are named `map_aruco_<id>`, where `<id>` represents the specific marker identifier.

```{hint}
The origin of a frame is located at the intersection of the colored axes.
The arrows indicate the positive direction of each axis: X (red), Y (green), Z (blue).

In accordance with [REP 103](https://reps.openrobotics.org/rep-0103/), the quadcopter's body frame is oriented as follows:
the X-axis points forward, the Y-axis points left, and the Z-axis points up. 
```
