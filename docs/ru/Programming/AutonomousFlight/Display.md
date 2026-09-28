# Дисплей

Клиент дисплея позволяет отправлять на подключённый экран сообщения ROS 2 типа `sensor_msgs.msg.Image`.

## Инициализация

```python
from clover2 import Clover2

drone = Clover2()
# или с именем ноды:
drone = Clover2("my_drone")

display = drone.display()
if display is None:
    raise RuntimeError("Драйвер дисплея недоступен")
```

`Clover2` — обёртка над ROS 2 Node. Внутри происходит вся магия: создаётся нода и запускается фоновый поток для работы ROS 2.

По умолчанию клиент подключается к драйверу по пути `/display`. Если драйвер находится по другому пути, передайте его в `display()`:

```python
display = drone.display("/my_display")
```

## Информация о драйвере

При создании клиента запрашиваются характеристики дисплея. Их нужно учитывать при подготовке изображения:

```python
print(f"Разрешение: {display.width}x{display.height}")
print(f"Максимальная частота: {display.max_fps} FPS")
print(f"Поддерживаемые кодировки: {display.supported_encodings}")
```

| Свойство | Описание |
| --- | --- |
| `valid` | `True`, если драйвер успешно вернул информацию о себе |
| `width` | ширина изображения в пикселях |
| `height` | высота изображения в пикселях |
| `max_fps` | максимальная частота отправки кадров |
| `supported_encodings` | список поддерживаемых кодировок ROS-изображений, например `mono8` |

:::{attention}
Размер кадра должен точно совпадать с `display.width` и `display.height`, а `image.encoding` — присутствовать в `display.supported_encodings`. Не отправляйте кадры чаще, чем позволяет `display.max_fps`.
:::

## Отправка изображения

Подготовьте объект `sensor_msgs.msg.Image` и передайте его в `send_image()`:

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

В примере используется `mono8`: один байт яркости на каждый пиксель, где `0` — чёрный, а `255` — белый. Текущий дисплей SSD1306 физически монохромный и отображает только чёрный и белый цвета, поэтому перед отправкой изображения с полутонами его нужно бинаризовать. Перед использованием этой кодировки убедитесь, что она есть в `display.supported_encodings`.

## Пример: вывести JPEG или PNG

Драйвер принимает сообщение `sensor_msgs.msg.Image`: его поле `data` содержит сырые пиксельные данные, а поле `encoding` задаёт их формат. OpenCV декодирует JPEG- или PNG-файл в `numpy.ndarray`; подготовленный массив можно передать в `send_cv_image()`, который преобразует его в ROS-сообщение с помощью `CvBridge`.

```python
import cv2

from clover2 import Clover2


drone = Clover2()
display = drone.display()

if display is None or not display.valid:
    raise RuntimeError("Драйвер дисплея недоступен")

if "mono8" not in display.supported_encodings:
    raise RuntimeError(
        f"Дисплей не поддерживает mono8: {display.supported_encodings}"
    )

frame = cv2.imread("image.jpg")  # также можно указать путь к PNG
if frame is None:
    raise RuntimeError("Не удалось загрузить изображение")

# SSD1306 отображает только чёрный и белый цвета.
gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
gray = cv2.resize(gray, (display.width, display.height))
_, binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY)

display.send_cv_image(binary, encoding="mono8")
```

Значение `127` — порог бинаризации: пиксели темнее него станут чёрными, а остальные — белыми. Подберите порог для конкретного изображения при необходимости.

`send_cv_image()` не изменяет размер и не преобразует кодировку изображения: перед отправкой массив должен иметь разрешение `display.width` × `display.height`, а указанная `encoding` должна поддерживаться дисплеем.

## Пример: тестовое изображение

Программа отправляет белую рамку и диагонали на чёрном фоне:

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
    raise RuntimeError("Драйвер дисплея недоступен")

if "mono8" not in display.supported_encodings:
    raise RuntimeError(
        f"Дисплей не поддерживает mono8: {display.supported_encodings}"
    )

display.send_image(make_test_image(display.width, display.height))
```