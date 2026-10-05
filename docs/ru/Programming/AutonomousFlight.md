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

## **{doc}`Полет <AutonomousFlight/Flight>`**

- `arm()` / `disarm()` — запуск / остановка электродвигателей
- `land()` — посадка
- `is_armed()` / `flight_mode()` — состояние квадрокоптера
- `navigate_wait(frame_id, x, y, z, speed, yaw)` — полет в заданную точку с ожиданием прибытия
- `navigate(...)` — полет в заданную точку без ожидания завершения команды

## **{doc}`Камера <AutonomousFlight/Camera>`**

- `get_image(camera_name, encoding)` — получение изображения с камеры в виде массива NumPy
- `get_image_msg(camera_name)` — получение исходного сообщения ROS Image
- `get_camera_info(camera_name)` — получение данных о калибровке камеры

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
