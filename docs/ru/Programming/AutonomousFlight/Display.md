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

В примере используется `mono8`: один байт яркости на каждый пиксель, где `0` — чёрный, а `255` — белый. Перед использованием этой кодировки убедитесь, что она есть в `display.supported_encodings`.

## Пример: вывести JPEG или PNG

Драйвер принимает не сжатые байты файла JPEG/PNG, а декодированное ROS-сообщение `sensor_msgs.msg.Image`. С помощью OpenCV можно загрузить файл, привести его к поддерживаемой кодировке и разрешению дисплея, а `CvBridge` преобразует numpy-массив в ROS-сообщение.

```python
import cv2
from cv_bridge import CvBridge

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

# Текущий драйвер SSD1306 принимает монохромные изображения mono8.
gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
gray = cv2.resize(gray, (display.width, display.height))

image = CvBridge().cv2_to_imgmsg(gray, encoding="mono8")
display.send_image(image)
```

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