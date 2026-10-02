# Thermal Imaging Camera

You can connect a thermal imaging camera to the Raspberry Pi 5 on a quadcopter to build thermal images and determine precise temperatures at individual points within the frame.

## Principle of Operation

The thermal imaging camera utilizes an infrared sensor with a resolution of 256×192 pixels. 
The camera transmits a single raw frame measuring 256×384 pixels over USB, which consists of two distinct sections:
* Top 192 rows: Infrared (IR) image data
* Bottom 192 rows: Temperature data (temperature matrix)

This raw data can be processed by a program. 
The top section can be converted into a visual image by applying a color palette. 
The bottom section (temperature matrix) can be converted into usable temperature values for analysis.

```{tip}
To obtain the temperature in Kelvin, divide the original raw value by 64. 
To convert this to degrees Celsius, subtract 273.15.
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

### Launching via v4l2_camera

The recommended method for launching the camera is using the `v4l2_camera` package. This approach transmits frames to ROS 2 without converting the original format to RGB.
It is critical to preserve the original `yuv422_yuy2`format.
Converting the data to a standard color image (RGB) may result in the loss of the temperature matrix data contained in the lower half of the frame.

Launch the node using the following command:

```bash
ros2 run v4l2_camera v4l2_camera_node --ros-args \
  -p video_device:=/dev/thermal_camera \
  -p image_size:="[256, 384]" \
  -p pixel_format:=YUYV \
  -p output_encoding:=yuv422_yuy2 \
  -p camera_frame_id:=thermal_camera \
  -r /image_raw:=/thermal_camera/image_raw \
  -r /camera_info:=/thermal_camera/camera_info
```

### Parameter Description

| Parameter | Value | Explanation |
|---|---|---|
| `video_device` | `/dev/thermal_camera` | Stable udev link to the thermal imager's Video4Linux device |
| `image_size` | `[256, 384]` | Full raw frame size (Top 192 rows: IR image; Bottom 192 rows: temperature matrix) |
| `pixel_format` | `YUYV` | Pixel format output by the USB camera |
| `output_encoding` | `yuv422_yuy2` | ROS 2 message encoding that preserves original data without RGB conversion |
| `camera_frame_id` | `thermal_camera` | `frame_id` name in the `sensor_msgs/msg/Image` header |
| `-r /image_raw:=...` | `/thermal_camera/image_raw` | Remapping of the image topic |
| `-r /camera_info:=...` | `/thermal_camera/camera_info` | Remapping of the calibration data topic |

## How to Check Functionality

To verify the camera is working correctly, open a new terminal and run the following commands:
1. Confirm the topic is active:

```bash
ros2 topic list | grep thermal_camera
```

2. Verify the message type:

```bash
ros2 topic info /thermal_camera/image_raw
```

Expected Output:

```text
Type: sensor_msgs/msg/Image
```

3. Check encoding, step, and frequency:

```bash
ros2 topic echo --once /thermal_camera/image_raw --field encoding
ros2 topic echo --once /thermal_camera/image_raw --field step
ros2 topic hz /thermal_camera/image_raw
```

Expected Values:

```text
encoding: yuv422_yuy2
step: 512
rate: approx. 25 Hz
```

## Code Examples

The following examples subscribe to the `/thermal_camera/image_raw` topic and do not utilize ROS parameters for topic remapping.
To run the examples, navigate to the directory containing the Python files and execute the following commands:

```bash
python3 subscribe_raw_image.py
python3 find_temperature_extremes.py
python3 visualize_raw_thermal.py
```

Examples' Overview:

1. `subscribe_raw_image.py` subscribes to the raw frame and publishes status updates to the `/thermal_camera/status` topic.
2. `visualize_raw_thermal.py` processes the top half of the raw frame and publishes a color-mapped version to `/thermal_camera/image_colormap`.
3. `find_temperature_extremes.py` analyzes the temperature matrix and publishes the minimum, maximum, and center temperatures to the following topics `/thermal_camera/min_temperature`, `/thermal_camera/max_temperature`, `/thermal_camera/center_temperature`. Extreme values are published as `geometry_msgs/msg/PointStamped` messages:
```text
point.x: Pixel coordinate along the horizontal axis
point.y: Pixel coordinate along the vertical axis
point.z: Temperature in degrees Celsius
```

### Splitting the Raw Frame

The raw frame has a resolution of `256x384` pixels with `yuv422_yuy2` encoding:

```text
/thermal_camera/image_raw
sensor_msgs/msg/Image 256x384, yuv422_yuy2

          256 px
     ┌──────────────┐
192  │ rows 0..191  │ IR image (for visualization)
px   ├──────────────┤
192  │ rows 192..383│ Temperature matrix
px   └──────────────┘
```

The top half is utilized for visual representation and `colormap` overlays. 
The bottom half is processed as `uint16` data and converted into degrees Celsius.

```python
raw = np.frombuffer(msg.data[:msg.height * msg.width * 2], dtype="<u2").reshape(msg.height, msg.width)
temperature_raw = raw[msg.height // 2:, :]
temperature_c = temperature_raw.astype(np.float32) / 64.0 - 273.15
```
