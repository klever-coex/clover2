# Тепловизионная камера

Mileseey TR256i — это портативный инфракрасный тепловизор (тепловизионная камера), подключаемый через USB Type-C.


## Принцип работы

Принцип работы Mileseey TR256i заключается в использовании теплового сенсора с разрешением 256×192 пикселя, который передаёт по USB 16-битный кадр, объединяющий два потока данных: инфракрасный снимок в градациях серого (GrayScale, яркость пикселя пропорциональна интенсивности ИК‑излучения) и матрицу температур (каждая точка данной матрицы содержит значение в виде 1/64 Кельвина). 
Полученные данные программно разделяются на два отдельных кадра, после чего каждый обрабатывается параллельно: инфракрасный снимок окрашивается в выбранную цветовую палитру, а температурная матрица преобразуется из 1/64 Кельвина в значения в градусах Цельсия.



## Сборка и установка модуля

Тепловизионную камеру можно установить на Клевер 5 двумя способами.

### Способ 1 (фронтальный):
1. Установите тепловизионную камеру в маунт для установки на защиту.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-front.webp
   :alt: Установка тепловизионной камеры в маунт для установки на защиту
   :width: 700px
   :align: center

   Рисунок 1 — Установка тепловизионной камеры в маунт для установки на защиту
   ```

2. Закрепите маунт с установленной камерой на лучи защиты, как показано на рисунке 2.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-1.webp
   :alt: Установка тепловизионной камеры на карбоновые лучи защиты
   :width: 700px
   :align: center

   Рисунок 2 — Установка тепловизионной камеры на карбоновые лучи защиты
   ```

3. Подсоедините USB-провод из комплекта к камере.
4. Вставьте ответный провод в свободный USB разъем на Raspberry Pi 5.
5. Зафиксируйте подключенный провод с помощью стяжек таким образом, чтобы он не попадал в область вращения пропеллеров.      

### Способ 2 (горизонтальный):

1. Установите тепловизионную камеру в маунт для установки в горизонтальное положение.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-down.webp
   :alt: Установка тепловизионной камеры в маунт для установки в горизонтальное положение
   :width: 700px
   :align: center

   Рисунок 3 — Установка тепловизионной камеры в маунт для установки в горизонтальное положение
   ```

2. С помощью винтов М3х8 закрепите маунт к нижней деке как показано на рисунке 4.

   ```{figure} @assets@/ru/programming/sensors/thermal-camera/thermal-mount-2.webp
   :alt: Установка камеры на нижнюю деку
   :width: 700px
   :align: center

   Рисунок 4 — Установка камеры на нижнюю деку
   ```

3. Подсоедините USB провод из комплекта к камере.

4. Вставьте ответный провод в свободный USB разъем на Raspberry Pi 5.

5. Зафиксируйте подключенный провод с помощью стяжек таким образом, чтобы он не попадал в область вращения пропеллеров.


## Настройка

### Запуск через v4l2_camera

Рекомендуемый способ запуска — пакет `v4l2_camera`, так как он может публиковать кадр без преобразования в RGB. Для тепловизора важно сохранить исходный формат `yuv422_yuy2`, потому что нижняя половина кадра содержит матрицу температур.

```bash
ros2 run v4l2_camera v4l2_camera_node --ros-args \
  -p video_device:=/dev/thermal_camera \
  -p image_size:="[256, 384]" \
  -p pixel_format:=YUYV \
  -p output_encoding:=yuv422_yuy2 \
  -p camera_frame_id:=thermal_camera \
  -r /image_raw:=/thermal_camera/image_raw \
  -r /camera_info:=/thermal_camera/camera_info
```

### Описание параметров

| Параметр | Значение | Пояснение |
|---|---|---|
| `video_device` | `/dev/thermal_camera` | Стабильная udev-ссылка на video4linux-устройство тепловизора. |
| `image_size` | `[256, 384]` | Размер полного raw-кадра. Верхние 192 строки — ИК-изображение, нижние 192 строки — матрица температур. |
| `pixel_format` | `YUYV` | Формат пикселей, который отдает USB-камера. |
| `output_encoding` | `yuv422_yuy2` | ROS encoding без преобразования в RGB. Нужен для корректного чтения температурной матрицы. |
| `camera_frame_id` | `thermal_camera` | Имя frame_id в заголовке сообщения `sensor_msgs/msg/Image`. |
| `-r /image_raw:=...` | `/thermal_camera/image_raw` | Переименование (remap) топика изображения. |
| `-r /camera_info:=...` | `/thermal_camera/camera_info` | Переименование топика калибровочных данных. |

## Проверка работоспособности

В другом терминале проверьте, что топик появился:

```bash
ros2 topic list | grep thermal_camera
```

Проверьте тип сообщения:

```bash
ros2 topic info /thermal_camera/image_raw
```

Ожидаемый тип:

```text
Type: sensor_msgs/msg/Image
```

Проверьте encoding, шаг строки и частоту:

```bash
ros2 topic echo --once /thermal_camera/image_raw --field encoding
ros2 topic echo --once /thermal_camera/image_raw --field step
ros2 topic hz /thermal_camera/image_raw
```

Ожидаемые значения:

```text
encoding: yuv422_yuy2
step: 512
rate: около 25 Гц
```

## Примеры кода

Примеры подписываются на `/thermal_camera/image_raw` и не используют ROS-параметры для смены топиков.

```bash
python3 subscribe_raw_image.py
python3 find_temperature_extremes.py
python3 visualize_raw_thermal.py
```

Назначение примеров:

```text
subscribe_raw_image.py — подписывается на raw-кадр и публикует строку /thermal_camera/status
find_temperature_extremes.py — публикует /thermal_camera/min_temperature, /thermal_camera/max_temperature, /thermal_camera/center_temperature
visualize_raw_thermal.py — берет верхнюю половину кадра и публикует /thermal_camera/image_colormap
```

`find_temperature_extremes.py` публикует точки `geometry_msgs/msg/PointStamped`:

```text
point.x — координата пикселя по горизонтали
point.y — координата пикселя по вертикали
point.z — температура в градусах Цельсия
```

### Разделение raw-кадра

Проверенный raw-кадр имеет размер `256x384` и encoding `yuv422_yuy2`:

```text
/thermal_camera/image_raw
sensor_msgs/msg/Image 256x384, yuv422_yuy2

          256 px
     ┌──────────────┐
192  │ rows 0..191  │  ИК-изображение для визуализации
px   ├──────────────┤
192  │ rows 192..383│  матрица температур
px   └──────────────┘
```

Верхняя половина используется для визуализации и наложения colormap. Нижняя половина читается как `uint16` и переводится в градусы Цельсия:

```python
raw = np.frombuffer(msg.data[:msg.height * msg.width * 2], dtype="<u2").reshape(msg.height, msg.width)
temperature_raw = raw[msg.height // 2:, :]
temperature_c = temperature_raw.astype(np.float32) / 64.0 - 273.15
```
