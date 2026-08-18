# LED-лента

## Инициализация

```python
from clover2 import Clover2

drone = Clover2()
# или с именем ноды:
drone = Clover2("my_drone")
```

`Clover2` - обёртка над ROS 2 Node. Внутри происходит вся магия: создаётся нода и запускается фоновый поток для работы ROS 2.

## Анимации

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
| `rainbow(period, brightness, duration)`        | `period` - время полного цикла радуги (по умолчанию 2.0)  |
| `blink(r, g, b, period, brightness, duration)` | `period` - период полного цикла on+off (по умолчанию 1.0) |
| `solid_color(r, g, b, brightness, duration)`   | заливка всей ленты одним цветом                           |
| `clear()`                                      | выключить все светодиоды                                  |

## Прямое управление

```python
# Заливка всей ленты
drone.fill(255, 255, 0)  # жёлтый
```

:::{attention}
При попиксельном управлении важно чтоб размер массива совпадал с колличеством светодиодов.
:::

```python
count = drone.led_count     # колличетсво диодов в ленте
leds = [(0, 0, 0)] * count  # выключаем ленту
leds[4] = (0, 255, 0)       # делаем 5 светодиод зеленым

# Попиксельное управление - [(r, g, b), ...]
drone.send_frame(leds, brightness=0.5)
```

## Пример: демонстрация эффектов

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
