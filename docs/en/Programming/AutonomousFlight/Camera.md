# Camera

The quadcopter's cameras allow you to capture high-quality images and integrate them directly into your custom computer vision programs.

## Initialization

To control the quadcopter, instantiate a `Clover2` object.

```python
from clover2 import Clover2

drone = Clover2()
# or with a specific node name:
drone = Clover2("my_drone")
```

`Clover2` serves as a wrapper around a ROS 2 node. Upon instantiation, the node is initialized, and a background thread is launched to handle ROS 2 operations.

## How to Retrieve a Frame as a NumPy Array

Use `get_image()` to retrieve the current camera frame as a NumPy array.

```python
img = drone.camera.get_image()                      # Default: retrieves from 'main_camera' in 'bgr8' format
img = drone.camera.get_image("main_camera", "rgb8") # Custom: specifying the camera name and encoding
```

If no parameters are provided, `get_image()` defaults to `main_camera` and `bgr8` format. You can customize the source and format using the `camera_name` and `encoding` parameters.

## How to Retrieve a ROS Image 

If you need to pass images directly through ROS topics, use `get_image_msg()`.

```python
img_msg = drone.camera.get_image_msg()
```

This method is more efficient if your program runs on ROS 2 and you do not need to immediately convert the image into a NumPy array. 

## How to Retrieve Camera Calibration Parameters

To get the camera calibration data, use `get_camera_info()`.

```python
info = drone.camera.get_camera_info()
# info.width, info.height, info.k (matrix), info.d (distortion)
```

## Example: QR Code Detection

This example demonstrates how to detect and decode a QR code in real-time.

```python
import cv2
from clover2 import Clover2

drone = Clover2()
detector = cv2.QRCodeDetector()

while True:
    img = drone.camera.get_image()
    data, bbox, _ = detector.detectAndDecode(img)
    if data:
        print(f"QR Code: {data}")
        break
```

`cv2.QRCodeDetector()` initializes a specialized detector object from the OpenCV library.
The `while True` loop continuously retrieves the latest frame from the camera using `img = drone.camera.get_image()`.
The `detectAndDecode()` function scans the frame for a QR code and attempts to extract the encoded text data.
Once a valid QR code is identified and its data is printed to the console, the `break` stops the capture process.
