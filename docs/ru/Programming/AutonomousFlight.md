# Автономный полет

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

AutonomousFlight/Flight
AutonomousFlight/LED
AutonomousFlight/Camera
```

Краткий обзор возможностей фреймворка.

## **{doc}`Полёт <AutonomousFlight/Flight>`**

- `arm()` / `disarm()` - запуск / остановка моторов
- `land()` - посадка
- `is_armed()` / `flight_mode()` - состояние дрона
- `navigate_wait(frame_id, x, y, z, speed, yaw)` - полёт в точку с ожиданием прибытия
- `navigate(...)` - то же без блокировки

## **{doc}`Камера <AutonomousFlight/Camera>`**

- `get_image(camera_name, encoding)` - получить кадр как numpy-массив
- `get_image_msg(camera_name)` - получить сырой ROS Image
- `get_camera_info(camera_name)` - калибровка камеры

## **{doc}`LED-лента <AutonomousFlight/LED>`**

- `rainbow(period, brightness, duration)` - анимация радуги
- `blink(r, g, b, period, brightness, duration)` - мигание
- `solid_color(r, g, b, brightness, duration)` - заливка одним цветом
- `clear()` - выключить ленту
- `fill(r, g, b)` - заливка прямым кадром
- `send_frame(colors, brightness)` - попиксельное управление
- `led_count` - количество светодиодов в ленте
