# Камера

## Инициализация

```python
from clover2 import Clover2

drone = Clover2()
# или с именем ноды:
drone = Clover2("my_drone")
```

`Clover2` — обёртка над ROS 2 Node. Внутри происходит вся магия: создаётся нода и запускается фоновый поток для работы ROS 2.

## Получить кадр как numpy-массив

```python
img = drone.camera().get_image()                       # main_camera, bgr8
img = drone.camera("main_camera").get_image("rgb8")  # с указанием камеры и encoding
```

## Получить ROS Image msg

```python
img_msg = drone.camera().get_image_msg()
```

## Получить калибровку камеры

```python
info = drone.camera().get_camera_info()
# info.width, info.height, info.k (матрица), info.d (дисторсия)
```

## Получать поток изображений

`stream` устанавливает callback, который вызывается для каждого нового кадра как
`numpy.ndarray`. По умолчанию используется кодировка `bgr8`; её можно изменить
аргументом `desired_encoding`.

```python
import numpy as np


def on_image(img: np.ndarray):
    print(img.shape)


drone.camera().stream(on_image)
```

Для получения исходных ROS-сообщений `sensor_msgs.msg.Image` используйте `stream_msg`:

```python
from sensor_msgs.msg import Image


def on_image_msg(msg: Image):
    print(msg.header.stamp)


drone.camera().stream_msg(on_image_msg)
```

Повторный вызов `stream` или `stream_msg` заменяет предыдущий callback. Подписка
продолжает работать до завершения `Clover2`.

## Пример: детекция QR-кода

```python
import cv2
from clover2 import Clover2

drone = Clover2()
detector = cv2.QRCodeDetector()

while True:
    img = drone.camera().get_image()
    data, bbox, _ = detector.detectAndDecode(img)
    if data:
        print(f"QR Code: {data}")
        break
```
