# Автономный полет

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

AutonomousFlight/Flight
AutonomousFlight/LED
AutonomousFlight/Camera
AutonomousFlight/Display
```

Краткий обзор возможностей фреймворка.

## **{doc}`Полёт <AutonomousFlight/Flight>`**

- `offboard.arm()` / `offboard.disarm()` — запуск / остановка моторов
- `offboard.land()` — посадка
- `fcu.is_armed()` / `fcu.flight_mode()` — состояние дрона
- `offboard.navigate_wait(frame_id, x, y, z, speed, yaw)` — полёт в точку с ожиданием прибытия
- `offboard.navigate(...)` — запускает полёт и возвращает `NavigationTask`
  - `task.status` — состояние задачи: `PENDING`, `ACTIVE`, `CANCELING`, `REJECTED`, `SUCCEEDED`, `CANCELED` или `ABORTED`
  - `task.wait(timeout=None)` — дождаться результата; возвращает `True` при успешном прибытии
  - `task.cancel()` — штатно отменить текущую навигационную цель, без посадки
  - `task.result` / `task.message` — результат action и сообщение bridge
  - `NavigationTimeoutError` — истекло время локального ожидания; полёт продолжается
  - `NavigationRejectedError` — bridge не принял новую цель
  - `NavigationCanceledError` — цель была отменена
  - `NavigationAbortedError` — навигация прервана ошибкой либо action server недоступен

`wait(timeout=...)` ограничивает только ожидание результата и не отменяет полёт. Для штатной остановки текущей навигации вызовите `task.cancel()`.

## **{doc}`Камера <AutonomousFlight/Camera>`**

- `camera(name)` — получить клиент камеры
- `get_image(encoding)` — получить кадр как numpy-массив
- `get_image_msg()` — получить сырой ROS Image
- `get_camera_info()` — калибровка камеры
- `stream(callback)` — получать новые ROS-сообщения `Image`

## **{doc}`LED-лента <AutonomousFlight/LED>`**

- `rainbow(period, brightness, duration)` — анимация радуги
- `blink(r, g, b, period, brightness, duration)` — мигание
- `solid_color(r, g, b, brightness, duration)` — заливка одним цветом
- `clear()` — выключить ленту
- `fill(r, g, b)` — заливка прямым кадром
- `send_frame(colors, brightness)` — попиксельное управление
- `led_count` — количество светодиодов в ленте

## **{doc}`Дисплей <AutonomousFlight/Display>`**

- `display()` — получить клиент дисплея
- `send_image(image)` — отправить ROS-сообщение `sensor_msgs.msg.Image`
- `send_cv_image(image, encoding)` — отправить изображение OpenCV / numpy-массив
- `width` / `height` — требуемое разрешение кадра
- `max_fps` — максимальная частота кадров
- `supported_encodings` — поддерживаемые кодировки изображений
