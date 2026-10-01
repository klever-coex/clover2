# LED strip

The LED strip allows you to control the quadcopter's LEDs using Python. 
You can execute pre-made animations, set a single color for the entire strip, or control each LED individually for custom patterns.

## Initialization

To control the quadcopter, instantiate a `Clover2` object.

```python
from clover2 import Clover2

drone = Clover2()
# or with a specific node name:
drone = Clover2("my_drone")
```

`Clover2` serves as a wrapper around a ROS 2 node. Upon instantiation, the node is initialized, and a background thread is launched to handle ROS 2 operations.

## Animations

The simplest way to use the LED strip is through built-in animations. Each animation supports a `duration` parameter, which specifies how long the animation should run in seconds.
If no `duration` is specified, the animation will run indefinitely until you start a new animation or send a direct command to the strip.

```python
drone.led().rainbow(period=2.0, duration=5.0)     # Rainbow effect: 2s per cycle, runs for 5s
drone.led().blink(255, 0, 0, period=0.5)          # Red flashing: 0.5s per on/off cycle
drone.led().solid_color(0, 255, 0, duration=1.0)  # Solid green for 1 second
drone.led().clear()                               # Turns off all LEDs
```

`period` defines the duration of a single animation cycle.
`duration` defines the total runtime of the animation effect.
For example, `led.rainbow(period=2.0, duration=5.0)` means one full color cycle takes 2 seconds, and the effect will play for a total of 5 seconds.

Animation Methods:

| Method                                         | Arguments and Descriptions                                        |
| ---------------------------------------------- | --------------------------------------------------------- |
| `rainbow(period, brightness, duration)`        | `period` is a continuous rainbow cycle (default: 2.0)  |
| `blink(r, g, b, period, brightness, duration)` | `period` flashes a specific color (default: 1.0) |
| `solid_color(r, g, b, brightness, duration)`   | Fills the entire strip with one color                           |
| `clear()`                                      | Turns off all LEDs                             |

## Direct Control

Direct control allows you to set a single color for the entire strip simultaneously.

```python
# Fill the entire strip
drone.led().fill(255, 255, 0)  # yellow
```

To achieve more complex patterns, you can use per-pixel control. This requires creating an array where each element represents the color of a specific LED.

:::{attention}
When using per-pixel control, the number of elements in your color array must exactly match the total number of LEDs in the strip.
:::

```python
count = drone.led().led_count   # Get the total number of LEDs in the strip
leds = [(0, 0, 0)] * count      # Set all LEDs to off
leds[4] = (0, 255, 0)           # Set the 5th LED to green

# Format: [(r, g, b), ...]
drone.led().send_frame(leds, brightness=0.5)
```

Each element in the leds list specifies an individual LED's color using the (r, g, b) format. 

`Python` uses zero-based indexing, so `leds[4]` refers to the fifth LED on the strip.

## Example: Demonstrating Effects

In this example, the program executes several LED effects in sequence: a rainbow animation, a blinking effect, and a three-color transition.

```python
import time
from clover2 import Clover2

drone = Clover2()

drone.led().rainbow(period=2.0, duration=5.0)
time.sleep(5.5)

drone.led().blink(255, 255, 255, period=0.5, duration=5.0)
time.sleep(5.5)

for r, g, b in [(255, 0, 0), (0, 255, 0), (0, 0, 255)]:
    drone.led().solid_color(r, g, b, duration=1.0)
    time.sleep(1.2)

drone.led().clear()
```

The `duration=5.0` parameter defines how long the animation plays. The `time.sleep(5.5)` pauses the script to ensure the animation completes before the program moves to the next command.

After the final color transition is complete, the program calls `clear()` to turn off all LEDs.
