# Clover2. Программирование

<br>

## Инициализация

```python
from clover2 import Clover2

drone = Clover2()
# или с именем ноды:
drone = Clover2("my_drone")
```

`Clover2` — обёртка над ROS 2 Node. Внутри происходит вся магия: создаётся нода и запускается фоновый поток для работы ROS 2.

## Полёт

### Взлёт и посадка

```python
drone.arm()          # запуск моторов
drone.disarm()       # остановка моторов
drone.land()         # посадка

drone.is_armed()     # True/False
drone.flight_mode()  # режим полёта PX4
```

### Полёт по точкам

Основной метод для полёта — функция которая работает, пока дрон не долетит до указанной точки. Первый аргумент — система координат, относительно которой задаётся точка. В примере ниже дрон полетит в точку 1, 2, 1.5 относительно начала карты со скоростью 0.5 метров в секунду.

```python
drone.navigate_wait(frame_id="map", x=1.0, y=2.0, z=1.5, speed=0.5, yaw=0.0)
```

В примере ниже дрон поднимется на высоту 50 см относительно текущего положения.

```python
drone.navigate_wait(frame_id="base_link", z=0.5, speed=0.5)
```

Метод navigate полностью копирует navigate_wait, но не будет ждать, пока дрон долетит до указанной точки.

```python
drone.navigate(frame_id="map", x=1.0, y=2.0, z=1.5, yaw=0.0, speed=0.5)
```

### Пример: полёт по квадрату

```python
import time
from clover2 import Clover2

drone = Clover2()

NAN = float("nan")
square_points = [(NAN, 2), (2, NAN), (NAN, -2), (-2, NAN)]

time.sleep(1)
drone.navigate_wait("base_link", z=1, speed=1.0)

for x, y in square_points:
    time.sleep(1)
    drone.navigate_wait("base_link", x=x, y=y, speed=0.8)

time.sleep(5.0)
drone.land()
```

## Камера

### Получить кадр как numpy-массив

```python
img = drone.get_image()                      # main_camera, bgr8
img = drone.get_image("main_camera", "rgb8") # с указанием камеры и encoding
```

### Получить ROS Image msg

```python
img_msg = drone.get_image_msg()
```

### Получить калибровку камеры

```python
info = drone.get_camera_info()
# info.width, info.height, info.k (матрица), info.d (дисторсия)
```

### Пример: детекция QR-кода

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

## LED-лента

### Анимации

Самый простой способ управления лентой - это встроенные анимации. Каждая анимация принимает `duration` (секунд); если значение не указано, то работает пока не будет заменена другой анимацией или прямой командой.

```python
drone.rainbow(period=2.0, duration=5.0)     # радуга, полный круг за 2с, 5 секунд
drone.blink(255, 0, 0, period=0.5)          # красное мигание, период 0.5с
drone.solid_color(0, 255, 0, duration=1.0)  # зелёная заливка на 1 секунду
drone.clear()                               # выключить все светодиоды
```

Параметры анимаций:

| Метод                                          | Аргументы                                                 |
| ---------------------------------------------- | --------------------------------------------------------- |
| `rainbow(period, brightness, duration)`        | `period` — время полного цикла радуги (по умолчанию 2.0)  |
| `blink(r, g, b, period, brightness, duration)` | `period` — период полного цикла on+off (по умолчанию 1.0) |
| `solid_color(r, g, b, brightness, duration)`   | заливка всей ленты одним цветом                           |
| `clear()`                                      | выключить все светодиоды                                  |

### Прямое управление

```python
# Заливка всей ленты
drone.fill(255, 255, 0)  # жёлтый
```

:::{attention}
При попиксельном управлении важно чтоб размер массива совпадал с колличеством светодиодов.
:::

```python
count = drone.led_count # колличетсво диодов в ленте
leds = [(0, 0, 0)] * count
leds[4] = (0, 255, 0)

# Попиксельное управление - [(r, g, b), ...] 
drone.send_frame(leds, brightness=0.5)
```

### Пример: демонстрация эффектов

```python
import time
from clover2 import Clover2

drone = Clover2()

drone.rainbow(period=2.0, duration=5.0)
time.sleep(5.5)

drone.blink(255, 255, 255, period=0.5, duration=5.0)
time.sleep(5.5)

for r, g, b in [(255, 0, 0), (0, 255, 0), (0, 0, 255)]:
    drone.solid_color(r, g, b, duration=1.0)
    time.sleep(1.2)

drone.clear()
```
