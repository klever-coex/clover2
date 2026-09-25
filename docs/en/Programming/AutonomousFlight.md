# Autonomous Flight: How to Program Your Quadcopter

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

AutonomousFlight/Flight
AutonomousFlight/LED
AutonomousFlight/Camera
```

This guide introduces the core framework features designed for programming autonomous quadcopter flight. 
Use these functions, whether you are developing custom control algorithms or building complex mission profiles.

Below is a quick reference to the framework's capabilities, organized by module.

## **{doc}`Flight Control <AutonomousFlight/Flight>`**

- `arm()` / `disarm()` — Motors start/stop
- `land()` — Auto-land
- `is_armed()` / `flight_mode()` — Status check
- `navigate_wait(frame_id, x, y, z, speed, yaw)` — Targeted flight (blocking)
- `navigate(...)` — Targeted flight (non-blocking)

## **{doc}`Camera <AutonomousFlight/Camera>`**

- `get_image(camera_name, encoding)` — Retrieves a camera frame converted into a NumPy array
- `get_image_msg(camera_name)` — Retrieves the raw ROS Image message 
- `get_camera_info(camera_name)` — Retrieves essential calibration data

## **{doc}`LED Strip <AutonomousFlight/LED>`**

- `rainbow(period, brightness, duration)` — Triggers a continuous rainbow animation
- `blink(r, g, b, period, brightness, duration)` — Makes the LEDs flash a specific color
- `solid_color(r, g, b, brightness, duration)` — Sets the LEDs to a single, steady color
- `clear()` — Turns off the LED strip 
- `fill(r, g, b)` — Fills the entire LED strip with a chosen color
- `send_frame(colors, brightness)` — Provides granular control (each individual LED with custom pattern)
- `led_count` — Displays the number of LEDs available in your strip
