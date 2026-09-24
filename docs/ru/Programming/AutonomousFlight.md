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

- `fcu.arm()` / `fcu.disarm()` — запуск / остановка моторов
- `fcu.land()` — посадка
- `fcu.is_armed()` / `fcu.flight_mode()` — состояние дрона
- `offboard.navigate_wait(frame_id, x, y, z, speed, yaw)` — полёт в точку с ожиданием прибытия
- `offboard.navigate(...)` — то же без блокировки

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
