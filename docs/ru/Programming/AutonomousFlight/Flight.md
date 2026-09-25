# Полёт

## Инициализация

```python
from clover2 import Clover2

drone = Clover2()
# или с именем ноды:
drone = Clover2("my_drone")
```

`Clover2` — обёртка над узлом ROS 2. Внутри создаётся нода и запускается фоновый поток для работы ROS 2.

## Полезные команды

```python
drone.offboard.arm()          # запуск моторов
drone.offboard.disarm()       # остановка моторов
drone.offboard.land()         # посадка

drone.fcu.is_armed()     # True/False
drone.fcu.flight_mode()  # режим полёта PX4
```

## Полёт по точкам

Основной метод для полёта — функция, которая работает, пока дрон не долетит до указанной точки. Первый аргумент — система координат, относительно которой задаётся точка. В примере ниже дрон полетит в точку 1, 2, 1,5 относительно начала карты со скоростью 0,5 метра в секунду.

```python
drone.offboard.navigate_wait(frame_id="map", x=1.0, y=2.0, z=1.5, speed=0.5, yaw=0.0)
```

В примере ниже дрон поднимется на высоту 50 см относительно текущего положения.

```python
drone.offboard.navigate_wait(frame_id="base_link", z=0.5, speed=0.5)
```

Метод `navigate` запускает полёт и сразу возвращает объект задачи. Через него можно дождаться окончания полёта или досрочно отменить текущую цель.

```python
from clover2.clients import NavigationCanceledError

task = drone.offboard.navigate(
    frame_id="map", x=1.0, y=2.0, z=1.5, yaw=0.0, speed=0.5
)

# ждать не более 10 секунд
# если timeout сработает, полёт не отменяется автоматически
task.wait(timeout=10.0)

# запросить штатную отмену action
task.cancel()

try:
    # ждать окончания
    task.wait()
except NavigationCanceledError:
    print("Полёт отменён")
```

Если `wait(timeout=...)` завершился по timeout, полёт не отменяется автоматически. Для остановки нужно явно вызвать `task.cancel()`.

## Пример: полёт по квадрату

```python
import time
from clover2 import Clover2

drone = Clover2()

NAN = float("nan")
square_points = [(NAN, 2), (2, NAN), (NAN, -2), (-2, NAN)]

time.sleep(1)
drone.offboard.navigate_wait("base_link", z=1, speed=1.0)

for x, y in square_points:
    time.sleep(1)
    drone.offboard.navigate_wait("base_link", x=x, y=y, speed=0.8)

time.sleep(5.0)
drone.offboard.land()
```

## Получение текущей координаты относительно карты

```python
print(drone.fcu.get_position())
```

Результат:
```python
DronePosition(x=-0.2, y=-0.05, z=1.0, roll=0.0, pitch=0.0, yaw=-0.4)
```

Также можно получить координату дрона относительно любой системы координат (например, конкретного маркера):
```python
print(drone.fcu.get_position("map_aruco_1"))
```

Результат:
```python
DronePosition(x=-1.0, y=-0.2, z=1.0, roll=0.0, pitch=0.0, yaw=1.17)
```
