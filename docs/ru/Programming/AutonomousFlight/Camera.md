# Камера

Камеры квадрокоптера позволяют получать изображения и использовать их в собственных программах.

## Инициализация

```python
from clover2 import Clover2

drone = Clover2()
# или с именем ноды:
drone = Clover2("my_drone")
```

`Clover2` — обертка над ROS 2 Node. При создании объекта Clover2 создается нода и запускается фоновый поток для работы ROS 2.

## Получение кадра в формате NumPy-массива

Используйте `get_image()`, чтобы получить текущий кадр с камеры в виде NumPy-массива.

```python
img = drone.camera().get_image()                       # main_camera, bgr8
img = drone.camera("main_camera").get_image("rgb8")  # с указанием камеры и encoding
```

Если не указать параметры, `get_image()` использует камеру `main_camera` и формат изображения `bgr8`.
Вы можете указать другую камеру и формат изображения с помощью параметров `camera_name` и `encoding`.

## Получение ROS Image 

Если вам нужно работать с изображением непосредственно в ROS 2, используйте `get_image_msg()`.

```python
img_msg = drone.camera().get_image_msg()
```

Используйте этот вариант, если ваша программа работает с ROS 2 и вам не нужно сразу преобразовывать изображение в NumPy-массив.

## Получение параметров калибровки камеры

Чтобы получить параметры калибровки камеры, используйте `get_camera_info()`.

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

В этом примере программа получает изображения с камеры и проверяет их на наличие QR-кода.

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

`cv2.QRCodeDetector()` создает детектор QR-кодов из библиотеки OpenCV.
В цикле `while True` программа постоянно получает кадры с камеры `img = drone.camera.get_image()`.
Функция `detectAndDecode()` ищет QR-код на полученном изображении и пытается прочитать записанные в нем данные.
После вывода данных команда `break` завершает цикл, и программа перестает получать новые кадры.
