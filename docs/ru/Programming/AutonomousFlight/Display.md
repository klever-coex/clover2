# Дисплей

Дисплей позволяет выводить изображения с программы на подключенный к квадрокоптеру экран. 

## Инициализация

Сначала создайте объект Clover2, а затем подключитесь к драйверу дисплея.

```python
from clover2 import Clover2

drone = Clover2()
# или с именем ноды:
drone = Clover2("my_drone")

display = drone.display()
if display is None:
    raise RuntimeError("Драйвер дисплея недоступен")
```

`Clover2` — обертка над ROS 2 Node. При создании объекта Clover2 создается нода и запускается фоновый поток для работы ROS 2.

По умолчанию клиент подключается к драйверу по пути `/display`. Если драйвер находится по другому пути, передайте его в `display()`:

```python
display = drone.display("/my_display")
```

## Информация о дисплее

При создании клиента программа получает характеристики дисплея. Используйте их при подготовке изображения:

```python
print(f"Разрешение: {display.width}x{display.height}")
print(f"Максимальная частота: {display.max_fps} FPS")
print(f"Поддерживаемые кодировки: {display.supported_encodings}")
```

Основные свойства клиента:

| Свойство | Описание |
| --- | --- |
| `valid` | `True`, если драйвер успешно вернул информацию о дисплее |
| `width` | ширина изображения в пикселях |
| `height` | высота изображения в пикселях |
| `max_fps` | максимальная частота отправки кадров |
| `supported_encodings` | список поддерживаемых кодировок ROS-изображений, например `mono8` |

:::{attention}
Размер кадра должен точно совпадать с `display.width` и `display.height`, а значение `image.encoding` должно входить в `display.supported_encodings`. Не отправляйте кадры чаще, чем позволяет `display.max_fps`.
:::

## Отправка изображения

Для отправки изображения создайте сообщение `sensor_msgs.msg.Image` и передайте его в `send_image()`.

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

В этом примере используется кодировка `mono8`. Она использует один байт для каждого пикселя: значение `0` соответствует черному цвету, а `255` — белому.
Дисплей SSD1306 физически отображает только черный и белый цвета. Поэтому изображение с полутонами перед отправкой нужно преобразовать в черно-белое.
Перед использованием `mono8` убедитесь, что эта кодировка есть в `display.supported_encodings`.


## Загрузка в формате JPEG или PNG

Вы можете загрузить изображение в формате `JPEG` или `PNG` с помощью `OpenCV`, преобразовать его в подходящий формат и отправить на дисплей.

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

# SSD1306 отображает только черный и белый цвета.
gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
gray = cv2.resize(gray, (display.width, display.height))
_, binary = cv2.threshold(gray, 127, 255, cv2.THRESH_BINARY)

display.send_cv_image(binary, encoding="mono8")
```

Программа выполняет следующие действия:
1.	Подключается к дисплею. 
2.	Проверяет, доступен ли драйвер. 
3.	Проверяет поддержку кодировки `mono8`. 
4.	Загружает изображение с помощью `OpenCV`. 
5.	Преобразует изображение в оттенки серого. 
6.	Изменяет размер изображения под разрешение дисплея. 
7.	Преобразует изображение в черно-белое. 
8.	Отправляет изображение на дисплей. 

Значение `127` в функции `cv2.threshold()` задает порог бинаризации. Пиксели со значением меньше `127` станут черными, а остальные — белыми. При необходимости измените это значение в зависимости от изображения.

`send_cv_image()` не изменяет размер и не преобразует кодировку изображения автоматически. Перед отправкой задайте размер массива `display.width` × `display.height` и используйте кодировку, которую поддерживает дисплей.

## Пример: тестовое изображение

Вы можете создать изображение непосредственно в программе и отправить его на дисплей. В этом примере программа рисует белую рамку и две диагонали на черном фоне.

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

Функция `make_test_image()` создает изображение размером с дисплей. Каждый пиксель хранится как отдельный байт: `0` соответствует черному цвету, а `255` — белому.
Полученное изображение можно использовать для проверки подключения и работы дисплея.
