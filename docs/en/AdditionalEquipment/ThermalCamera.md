# Thermal Imaging Camera User Guide

You can connect a thermal imaging camera to the Raspberry Pi 5 on a quadcopter to build thermal images and determine precise temperatures at individual points within the frame.

## Principle of Operation

A thermal imaging camera captures infrared radiation emitted from surrounding objects. The camera's sensor converts this radiation into electrical signals, which are then processed by onboard electronics to generate a thermal image. This image provides a visual representation of the temperature distribution across object surfaces.

The integrated thermal module features a resolution of 256x192 pixels and outputs two data streams: a processed thermal image and a temperature matrix.

## Data in ROS 2

The `clover2_thermal` driver splits the raw camera data and publishes it via two separate ROS 2 topics. By default, both topics utilize the `sensor_msgs/msg/Image` message type.

| Topic | Encoding | Content |
| --- | --- | --- |
| `/thermal_camera/image_viz` | `mono8` | Processed grayscale thermal image. Each pixel contains a brightness value from 0 to 255. |
| `/thermal_camera/temperature` | `32FC1` | Temperature matrix. Each pixel contains the exact temperature in degrees Celsius as a 32-bit floating-point number. |

Both images maintain a consistent size of 256×192 pixels. Pixel coordinates are identical across both topics. 

Use the `image_viz` topic for visual monitoring and thermal imaging. Access the `temperature` topic to obtain precise temperature measurements.

```{hint}
Do not rely on pixel brightness alone to determine temperature. The grayscale image is for visualization and does not represent exact numerical temperature values.
```

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

   <br>

2. Secure the mount (with the camera installed) onto the guard arcs, as shown in the figure below.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-1.webp
   :alt: Установка тепловизионной камеры на карбоновые лучи защиты
   :width: 700px
   :align: center

   Mount installed to the quadcopter guard arcs
   ```

   <br>

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

   <br>

2. Secure the mount to the bottom deck using M3x8 screws, as shown in the figure.


   ```{figure} @assets@/en/programming/sensors/thermal-camera/thermal-mount-2.webp
   :alt: Установка камеры на нижнюю деку
   :width: 700px
   :align: center

   Mount secured to the bottom deck
   ```

   <br>

3. Connect the USB cable from the kit to the camera.
4. Plug the other end of the USB cable into an available USB port on the Raspberry Pi 5.
5. Secure the cable using zip ties to ensure it remains clear of the propeller rotation area.

## Configuration

### Enabling via `clover2-settings`

To enable the thermal imaging camera on your quadcopter, update the system configuration using the `clover2-settings` utility.

1. Open a terminal and execute:

```bash
clover2-settings
```

2. Select the `additional_sensors` group.

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings.webp
:alt: Selecting the additional_sensors group in clover2-settings
:width: 700px
:align: center

Navigating to sensors
```

<br>

3. In the `additional_sensors` group, select the `thermal_camera` setting  and change its value to `true`.

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings-thermal-camera.webp
:alt: Selecting the thermal_camera setting
:width: 700px
:align: center

Enabling the camera
```

<br>

4. Press `Ctrl+S` to save the changes (a notifiaction will confirm the save).

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings-save.webp
:alt: Saving the thermal imaging camera setting
:width: 700px
:align: center

Saving
```

<br>

5. Press `Esc` several times to exit the application.
6. Restart the `clover2` service with the following command:

```bash
sudo systemctl restart clover2
```

Once the service has started, the thermal camera driver will automatically begin publishing data to the `/thermal_camera/image_viz` and `/thermal_camera/temperature` topics.

## Usage

For Python development, you can obtain the temperature matrix using the `Clover2` client:

```python
from clover2 import Clover2

drone = Clover2()
temperature = drone.thermal_camera.get_temperature()

# The NumPy matrix has shape (192, 256), with indices in [y, x] order.
print(f"Temperature at the center of the frame: {temperature[96, 128]:.2f} °C")
```

The pixel coordinate system originates from the top-left corner: `x`-axis increases from left to right, and `y`-axis increases  from top to bottom. 

The `get_temperature()` method returns the most recent temperature matrix as a NumPy array. On the initial call, the method will wait up to 5 seconds for data.  If no data received, a `TimeoutError` exception will be raised. 

To obtain the original ROS message instead of a processed NumPy matrix, use the `drone.thermal_camera.get_temperature_msg()` method.
