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

В этом разделе описаны основные возможности фреймворка для программирования автономного полета квадрокоптера. 
Используйте эти функции при создании собственных программ и алгоритмов управления квадрокоптером.

Краткий обзор возможностей фреймворка.

## **{doc}`Полёт <AutonomousFlight/Flight>`**
- `fcu.arm()` / `fcu.disarm()` — запуск / остановка электродвигателей
- `fcu.is_armed()` / `fcu.flight_mode()` — состояние квадрокоптера
- `offboard.land()` — посадка
- `offboard.navigate_wait(frame_id, x, y, z, speed, yaw)` — полет в заданную точку с ожиданием прибытия
- `offboard.navigate(...)` — полет в заданную точку без ожидания завершения команды, возвращает `NavigationTask`
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

- `camera(name)` — получение клиента камеры
- `get_image(encoding)` — получение изображения с камеры в виде массива NumPy
- `get_image_msg()` — получение исходного сообщения ROS Image
- `get_camera_info()` — получение данных о калибровке камеры
- `stream(callback)` — устанавливливание callback, который вызывается для каждого нового изображения с камеры в виде массива NumPy  
- `stream_msg(callback)` — устанавливливание callback, который вызывается для каждого нового сообщения ROS Image

## **{doc}`LED-лента <AutonomousFlight/LED>`**

- `rainbow(period, brightness, duration)` — запуск анимации с эффектом радуги
- `blink(r, g, b, period, brightness, duration)` — мигание светодиодами
- `solid_color(r, g, b, brightness, duration)` — включение одного цвета
- `clear()` — выключение LED-ленты
- `fill(r, g, b)` — заполнение ленты одним цветом
- `send_frame(colors, brightness)` — управление каждым светодиодом отдельно
- `led_count` — количество светодиодов в ленте

## **{doc}`Дисплей <AutonomousFlight/Display>`**

- `display()` — получить клиент дисплея
- `send_image(image)` — отправить ROS-сообщение `sensor_msgs.msg.Image`
- `send_cv_image(image, encoding)` — отправить изображение OpenCV / numpy-массив
- `width` / `height` — требуемое разрешение кадра
- `max_fps` — максимальная частота кадров
- `supported_encodings` — поддерживаемые кодировки изображений
