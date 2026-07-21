# Камера

## Инициализация

```python
from clover2 import Clover2

drone = Clover2()
# или с именем ноды:
drone = Clover2("my_drone")
```

`Clover2` - обёртка над ROS 2 Node. Внутри происходит вся магия: создаётся нода и запускается фоновый поток для работы ROS 2.

## Получить кадр как numpy-массив

```python
img = drone.get_image()                      # main_camera, bgr8
img = drone.get_image("main_camera", "rgb8") # с указанием камеры и encoding
```

## Получить ROS Image msg

```python
img_msg = drone.get_image_msg()
```

## Получить калибровку камеры

```python
info = drone.get_camera_info()
# info.width, info.height, info.k (матрица), info.d (дисторсия)
```

## Пример: детекция QR-кода

```python
import cv2
from clover2 import Clover2

drone = Clover2()
detector = cv2.QRCodeDetector()

while True:
    img = drone.get_image()
    data, bbox, _ = detector.detectAndDecode(img)
    if data:
        print(f"QR Code: {data}")
        break
```
