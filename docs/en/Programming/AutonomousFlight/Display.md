# Display

The display interface allows you to output images from your program to a screen connected to the quadcopter. 

## Initialization

To begin, create a `Clover2` object and then connect to the display driver.

```python
from clover2 import Clover2

drone = Clover2()
# or with a specific node name:
drone = Clover2("my_drone")

display = drone.display()
if display is None:
    raise RuntimeError("Display driver not available")
```

`Clover2` serves as a wrapper around a ROS 2 node. Upon instantiation, the node is initialized, and a background thread is launched to handle ROS 2 operations.

By default, the client connects to the driver at the path `/display`. 
If your driver is located at a different path, pass the path as an argument to `display()`:

```python
display = drone.display("/my_display")
```

## Display Information

When the client is initialized, the program automatically retrieves the display's specifications. 
It is recommended to use these properties to prepare your images correctly.

```python
print(f"Resolution: {display.width}x{display.height}")
print(f"Maximum frame rate: {display.max_fps} FPS")
print(f"Supported encodings: {display.supported_encodings}")
```

Key Client Properties:

| Property | Description |
| --- | --- |
| `valid` | `True`if the driver successfully returned display information |
| `width` | Image width in pixels |
| `height` | Image height in pixels |
| `max_fps` | Maximum supported frame rate |
| `supported_encodings` | List of supported ROS image encodings, e.g `mono8` |

:::{attention}
The frame size must exactly match `display.width` and `display.height`, and the `image.encoding` value must be included in `display.supported_encodings`. 
Do not send frames more frequently than the allowed  `display.max_fps`.
:::

## How to Send an Image

To send an image, create a `sensor_msgs.msg.Image` message and pass it to `send_image()`.

```python
from sensor_msgs.msg import Image

image = Image()
image.width = display.width
image.height = display.height
image.encoding = "mono8"
image.is_bigendian = False
image.step = image.width
image.data = bytearray(image.width * image.height)

display.send_image(image)
```

This example uses the `mono8` encoding, which uses one byte per pixel: a value of `0` corresponds to black, and `255` corresponds to white.
The SSD1306 display physically renders only black and white. Therefore, any images containing halftones must be converted to binary (black and white) before transmission. 
Always verify that `mono8` is listed in `display.supported_encodings` before use.

## How to Load JPEG or PNG Images

You can load an image in JPEG or PNG format using OpenCV, convert it to the appropriate format, and then send it to the display.

```python
import cv2

from clover2 import Clover2


drone = Clover2()
display = drone.display()

if display is None or not display.valid:
    raise RuntimeError("Display driver is not available")

if "mono8" not in display.supported_encodings:
    raise RuntimeError(
        f"Display does not support mono8: {display.supported_encodings}"
    )

frame = cv2.imread("image.jpg")  # Supports PNG as well
if frame is None:
    raise RuntimeError("Failed to load image")

# Pre-process for the SSD1306.
gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
gray = cv2.resize(gray, (display.width, display.height))
_, binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY)

display.send_cv_image(binary, encoding="mono8")
```

The program implements the following steps:
1. Connects to the display.
2. Validates that the driver is active and available.
3. Verifies that the `mono8` encoding is supported by the hardware.
4. Loads the image file using OpenCV.
5. Converts the image to grayscale.
6. Resizes the image to match the display's exact resolution.
7. Binarizes the image to convert it to pure black and white.
8. Transmits the processed image to the display.

The value `127` in `cv2.threshold()` defines the binarization threshold. 
Pixels with a brightness value lower than `127` will become black, while those above will become white. You can adjust this value to optimize the image contrast for your specific file.

The `send_cv_image()`  does not automatically resize images or convert encodings. 
You must ensure the array dimensions match  `display.width` × `display.height` and use a supported encoding prior to sending.

## Example: Test Image

You can generate an image directly within your program and send it to the display. 
In this example, the program draws a white border and two diagonal lines on a black background to verify the display's functionality.

```python
from clover2 import Clover2
from sensor_msgs.msg import Image


def make_test_image(width: int, height: int) -> Image:
    image = Image()
    image.width = width
    image.height = height
    image.encoding = "mono8"
    image.is_bigendian = False
    image.step = width

    pixels = bytearray(width * height)
    for y in range(height):
        for x in range(width):
            if (
                x == 0
                or x == width - 1
                or y == 0
                or y == height - 1
                or x == y
                or x == width - y - 1
            ):
                pixels[y * width + x] = 255

    image.data = pixels
    return image


drone = Clover2()
display = drone.display()

if display is None or not display.valid:
    raise RuntimeError("Display driver not available")

if "mono8" not in display.supported_encodings:
    raise RuntimeError(
        f"Display does not support mono8: {display.supported_encodings}"
    )

display.send_image(make_test_image(display.width, display.height))
```

The `make_test_image()` function creates an image matching the display's resolution. 
Each pixel is stored as a single byte, where `0` represents black and  `255` represents white.

The resulting image is an effective way to verify both the display's connection and its correct operation.
