# Flight

## Initialization

To control the quadcopter, instantiate a `Clover2` object.

```python
from clover2 import Clover2

drone = Clover2()
# or with a specific node name:
drone = Clover2("my_drone")
```

`Clover2` serves as a wrapper around a ROS 2 node. Upon instantiation, the node is initialized, and a background thread is launched to handle ROS 2 operations.

## Useful Commands

Use the following methods to control the quadcopter's state:

```python
drone.arm()          # Arms the motors to enable flight
drone.disarm()       # Disarms the motors to stop them
drone.land()         # Commands the quadcopter to land

drone.is_armed()     # Returns `True` if the motors are armed, `False` otherwise
drone.flight_mode()  # Retrieves the current PX4 flight mode
```

## Waypoint Flight

To navigate the quadcopter to a specific waypoint, use the `navigate_wait()` function. 
Specify the coordinate system using the `frame_id` parameter. Define the target position with `x`, `y` and `z`, and set the cruising velocity using the `speed` parameter.

The following example demonstrates the quadcopter flying at  0.5 m/s. The `navigate_wait()` function blocks execution until the quadcopter reaches the target coordinates, allowing the program to proceed only after the waypoint is reached.

```python
drone.navigate_wait(frame_id="map", x=1.0, y=2.0, z=1.5, speed=0.5, yaw=0.0)
```

In this instance, coordinates are defined relative to the `map` coordinate system.
To adjust altitude relative to the quadcopter's current position, use the `base_link` coordinate system. 

The following example commands the quadcopter to ascend 0.5 m from its current height:

```python
drone.navigate_wait(frame_id="base_link", z=0.5, speed=0.5)
```

Use `navigate_wait()` when subsequent command needs to be executed after the quadcopter has already reached the target point.

## Flight Without Waiting

The `navigate()` function operates similarly to `navigate_wait()`, but it is non-blocking. Once the command is sent, the program continues execution immediately without waiting for the quadcopter to reach the destination.

```python
drone.navigate(frame_id="map", x=1.0, y=2.0, z=1.5, yaw=0.0, speed=0.5)
```

Use `navigate()` when your program needs to perform other tasks while the quadcopter is flying.

## Example: Flying a Square

In this example, the quadcopter ascends to a height of 1 m and sequentially navigates through four points to trace a square pattern.

```python
import time
from clover2 import Clover2

drone = Clover2()

NAN = float("nan")
square_points = [(NAN, 2), (2, NAN), (NAN, -2), (-2, NAN)]

time.sleep(1)
drone.navigate_wait("base_link", z=1, speed=1.0)

for x, y in square_points:
    time.sleep(1)
    drone.navigate_wait("base_link", x=x, y=y, speed=0.8)

time.sleep(5.0)
drone.land()
```

The use of `float("nan")` creates a special `NaN` value to tell the controller not to change the specific axis.
The four coordinate pairs define the following sequential movements relative to the current position: 
1. `x` remains unchanged, `y` increases by 2 m
2. `x` increases by 2 m, `y` remains unchanged
3. `x` remains unchanged, `y` decreases by 2 m
4. `x` decreases by 2 m, `y` remains unchanged

By following these steps, the quadcopter traces the four sides of a square. After the final waypoint is reached, the program waits for 5 seconds and then initiates landing via the `land()`.

## How to Get Current Coordinates

To retrieve the quadcopter's current position relative to the map, use the `get_position()` function.

```python
print(drone.get_position())
```

Output:

```python
DronePosition(x=-0.2, y=-0.05, z=1.0, roll=0.0, pitch=0.0, yaw=-0.4)
```

## How to Get Coordinates Relative to Another Coordinate System

You can retrieve the quadcopter's current position relative to any available coordinate system. For instance, to get the coordinates relative to the `map_aruco_1` marker, specify its name:

```python
print(drone.get_position("map_aruco_1"))
```

Output:
```python
DronePosition(x=-1.0, y=-0.2, z=1.0, roll=0.0, pitch=0.0, yaw=1.17)
```

The specified coordinate system name must exist within the `TF` tree. If the system name is invalid, or if a transformation between the quadcopter and the target system cannot be calculated, the position cannot be retrieved.
