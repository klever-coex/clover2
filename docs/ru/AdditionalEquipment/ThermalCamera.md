# Тепловизионная камера

Mileseey TR256i — это портативный инфракрасный тепловизор, подключаемый через USB Type-C.


## Принцип работы

Принцип работы Mileseey TR256i заключается в использовании теплового сенсора с разрешением 256×192 пикселя, который передаёт по USB 16-битный кадр, объединяющий два потока данных: инфракрасный снимок в градациях серого (GrayScale, яркость пикселя пропорциональна интенсивности ИК‑излучения) и матрицу температур (каждая точка данной матрицы содержит значение в виде 1/64 Кельвина). 
Полученные данные программно разделяются на два отдельных кадра, после чего каждый обрабатывается параллельно: инфракрасный снимок окрашивается в выбранную цветовую палитру, а температурная матрица преобразуется из 1/64 Кельвина в значения в градусах Цельсия.



## Сборка и установка модуля

Тепловизионную камеру можно установить на Клевер 5 двумя способами.

### Способ №1 (фронтальный):
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
4. Вставьте ответный провод в свободный USB-разъем на Raspberry Pi 5.
5. Зафиксируйте подключенный провод с помощью стяжек таким образом, чтобы он не попадал в область вращения пропеллеров.      

### Способ №2 (горизонтальный):

1. Установите тепловизионную камеру в маунт для установки в горизонтальное положение.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-down.webp
   :alt: Установка тепловизионной камеры в маунт для установки в горизонтальное положение
   :width: 700px
   :align: center

   Рисунок 3 — Установка тепловизионной камеры в маунт для установки в горизонтальное положение
   ```


2. С помощью винтов М3х8 закрепите маунт к нижней деке, как показано на рисунке 4.

   ```{figure} @assets@/ru/programming/sensors/thermal-camera/thermal-mount-2.webp
   :alt: Установка камеры на нижнюю деку
   :width: 700px
   :align: center

   Рисунок 4 — Установка камеры на нижнюю деку
   ```

3. Подсоедините USB-провод из комплекта к камере.

4. Вставьте ответный провод в свободный USB-разъем на Raspberry Pi 5.

5. Зафиксируйте подключенный провод с помощью стяжек таким образом, чтобы он не попадал в область вращения пропеллеров.


## Настройка

### Запуск через `clover2-settings`

Чтобы включить тепловизионную камеру в Клевер необходимо открыть настройки:

```bash
clover2-settings
```

Выберите группу `additional_sensors`, как показано на рисунке 5.

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings.webp
:alt: Выбор группы additional_sensors в clover2-settings
:width: 700px
:align: center

Рисунок 5 — Выбор группы additional_sensors в clover2-settings
```

В группе `additional_sensors` выберите настройку `thermal_camera` (см. рисунок 6) и включите её, установив значение `true`.

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings-thermal-camera.webp
:alt: Выбор настройки thermal_camera
:width: 700px
:align: center

Рисунок 6 — Выбор пункта thermal_camera
```

Для сохранения изменений нажмите ctrl+S. Появится уведомление о сохранении, как показано на рисунке 7.

```{figure} @assets@/common/programming/sensors/thermal-camera/clover2-settings-save.webp
:alt: Сохранение настройки тепловизионной камеры
:width: 700px
:align: center

Рисунок 7 — Сохранение настройки тепловизионной камеры
```

Затем несколько раз нажмите esc чтоб выйти из приложения. Перезапустите сервис clover2:

```bash
sudo systemctl restart clover2
```

После успешного запуска драйвер будет публиковать кадры в топик `/thermal_camera/image_raw`. Размер полного raw-кадра — `256x384`, encoding — `yuv422_yuy2`. Верхние 192 строки содержат ИК-изображение, нижние 192 строки — матрицу температур.

## Проверка работоспособности

Проверьте, что топик появился:

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

Исходные файлы находятся в папке `clover2/examples/thermal_camera`. Их также можно скачать отдельно:

- {download}`subscribe_raw_image.py <../../../clover2/examples/thermal_camera/subscribe_raw_image.py>`
- {download}`find_temperature_extremes.py <../../../clover2/examples/thermal_camera/find_temperature_extremes.py>`
- {download}`visualize_raw_thermal.py <../../../clover2/examples/thermal_camera/visualize_raw_thermal.py>`

Для запуска установленных примеров перейдите в папку:

```bash
cd examples/thermal_camera
```
Запустите нужный пример:

```bash
python3 subscribe_raw_image.py
python3 find_temperature_extremes.py
python3 visualize_raw_thermal.py
```

Каждый пример работает до нажатия ctrl+C. Для одновременного запуска используйте отдельные терминалы.

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

В данном случае raw-кадр имеет размер `256x384` и encoding `yuv422_yuy2`:

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
