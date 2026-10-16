# Thermal Imaging Camera

You can connect a thermal imaging camera to the Raspberry Pi 5 on a quadcopter to build thermal images and determine precise temperatures at individual points within the frame.

## Principle of Operation

The thermal imaging camera detects infrared radiation emitted by surrounding objects. A dedicated sensor converts thermal radiation into electrical signals. The camera's electronics use these signals to determine the temperature distribution and produce a thermal image of the object.

The thermal imaging module used here has a resolution of 256x192 pixels and returns two frames: a processed thermal image and a temperature matrix.

## Data in ROS 2

The `clover2_thermal` driver splits the frame received from the camera into a thermal image and a temperature matrix. With the default settings, these are published to two topics with the `sensor_msgs/msg/Image` message type:

| Topic | Encoding (`encoding`) | Content |
| --- | --- | --- |
| `/thermal_camera/image_viz` | `mono8` | Processed grayscale thermal image. Each pixel contains a brightness value from 0 to 255. |
| `/thermal_camera/temperature` | `32FC1` | Temperature matrix. Each pixel contains a temperature in degrees Celsius as a 32-bit floating-point number. |

Both images measure 256×192 pixels. Each pixel in the thermal image corresponds to an element of the temperature matrix at the same coordinates. Use `image_viz` to view the image and `temperature` to obtain temperature values.

## How to Install the Camera

The thermal imaging camera can be mounted on the quadcopter using one of two methods.

### Method 1: Front-facing Mount
1. Install the thermal imaging camera into the provided mount.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-front.webp
   :alt: Установка тепловизионной камеры в маунт для установки на защиту
   :width: 700px
   :align: center
   
   Camera in the mount
   ```

2. Secure the mount (with the camera installed) onto the guard arcs, as shown in the figure below.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-1.webp
   :alt: Установка тепловизионной камеры на карбоновые лучи защиты
   :width: 700px
   :align: center

   Mount installed to the quadcopter guard arcs
   ```

3. Connect the USB cable from the kit to the camera.
4. Plug the other end of the USB cable into an available USB port on the Raspberry Pi 5.
5. Secure the cable using zip ties to ensure it remains clear of the propeller rotation area.      

### Method 2: Horizontal Mount

1. Install the thermal imaging camera into the provided mount.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-down.webp
   :alt: Установка тепловизионной камеры в маунт для установки в горизонтальное положение
   :width: 700px
   :align: center

   Camera in the mount
   ```

2. Secure the mount to the bottom deck using M3x8 screws, as shown in the figure.


   ```{figure} @assets@/ru/programming/sensors/thermal-camera/thermal-mount-2.webp
   :alt: Установка камеры на нижнюю деку
   :width: 700px
   :align: center

   Mount secured to the bottom deck
   ```

3. Connect the USB cable from the kit to the camera.
4. Plug the other end of the USB cable into an available USB port on the Raspberry Pi 5.
5. Secure the cable using zip ties to ensure it remains clear of the propeller rotation area.

## Configuration

### Launching via `clover2-settings`

To enable the thermal imaging camera in Clover, open the settings:

Launch the node using the following command:

```bash
clover2-settings
```

Select the `additional_sensors` group, as shown in Figure 5.

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings.webp
:alt: Selecting the additional_sensors group in clover2-settings
:width: 700px
:align: center

Figure 5 — Selecting the additional_sensors group in clover2-settings
```

In the `additional_sensors` group, select the `thermal_camera` setting (see Figure 6) and enable it by setting its value to `true`.

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings-thermal-camera.webp
:alt: Selecting the thermal_camera setting
:width: 700px
:align: center

Figure 6 — Selecting the thermal_camera option
```

Press Ctrl+S to save the changes. A confirmation will appear, as shown in Figure 7.

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings-save.webp
:alt: Saving the thermal imaging camera setting
:width: 700px
:align: center

Figure 7 — Saving the thermal imaging camera setting
```

Then press Esc several times to exit the application. Restart the clover2 service:

```bash
sudo systemctl restart clover2
```

Once the driver starts successfully, it will publish frames to the `/thermal_camera/image_viz` and `/thermal_camera/temperature` topics.

## Usage

In Python, you can obtain the temperature matrix using the `Clover2` client:

```python
from clover2 import Clover2

drone = Clover2()
temperature = drone.thermal_camera.get_temperature()

# The NumPy matrix has shape (192, 256), with indices in [y, x] order.
print(f"Temperature at the center of the frame: {temperature[96, 128]:.2f} °C")
```

The pixel coordinate origin is at the top-left corner: `x` increases to the right and `y` increases downward. The `get_temperature()` method returns the most recently received NumPy matrix. On the first call, it waits up to 5 seconds for data and raises `TimeoutError` if none arrives. To obtain the original ROS message, use `drone.thermal_camera.get_temperature_msg()`.
