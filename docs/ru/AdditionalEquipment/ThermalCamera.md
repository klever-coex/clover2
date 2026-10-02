# Тепловизионная камера

Вы можете подключить тепловизионную камеру к Raspberry Pi 5 на квадрокоптере и использовать полученные с нее данные для построения теплового изображения и определения температуры в отдельных точках кадра.

## Принцип работы

Тепловизионная камера использует инфракрасный сенсор с разрешением 256×192 пикселя. 
Камера передает по USB один raw-кадр размером 256×384 пикселя, который содержит две части:
* верхние 192 строки — данные инфракрасного изображения;
* нижние 192 строки — данные о температуре (матрица температур).

Полученные данные можно обрабатывать программно. Верхнюю часть кадра можно преобразовать в изображение и наложить цветовую палитру. Нижнюю часть (матрицу температур) можно преобразовать в значения температуры для анализа. 

```{tip}
Чтобы получить температуру в Кельвинах, разделите исходное значение на 64. 
Чтобы получить значение в градусах Цельсия, вычтите 273,15 из значения в Кельвинах.
```

## Сборка и установка модуля

Тепловизионную камеру можно установить на квадрокоптер двумя способами.

### Способ №1 (фронтальный):
1. Установите тепловизионную камеру в предусмотренное крепление для модуля камеры.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-front.webp
   :alt: Установка тепловизионной камеры в маунт для установки на защиту
   :width: 700px
   :align: center

   Установка тепловизионной камеры в крепление
   ```

2. Закрепите крепление с установленной камерой на дугах защиты, как показано на рисунке.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-1.webp
   :alt: Установка тепловизионной камеры на карбоновые лучи защиты
   :width: 700px
   :align: center

   Установка тепловизионной камеры на карбоновые дуги защиты
   ```

3. Подсоедините USB-кабель из комплекта к камере.
4. Вставьте второй конец USB-кабеля в свободный USB-разъем Raspberry Pi 5.
5. Зафиксируйте подключенный кабель стяжками так, чтобы он не попадал в область вращения пропеллеров.      

### Способ №2 (горизонтальный):

1. Установите тепловизионную камеру в предусмотренное крепление для модуля камеры.

   ```{figure} @assets@/common/programming/sensors/thermal-camera/thermal-mount-down.webp
   :alt: Установка тепловизионной камеры в маунт для установки в горизонтальное положение
   :width: 700px
   :align: center

   Установка тепловизионной камеры в крепление
   ```

2. С помощью винтов М3х8 закрепите крепление на нижней деке, как показано на рисунке.


   ```{figure} @assets@/ru/programming/sensors/thermal-camera/thermal-mount-2.webp
   :alt: Установка камеры на нижнюю деку
   :width: 700px
   :align: center

   Установка тепловизионной камеры на нижнюю деку
   ```

3. Подсоедините USB-кабель из комплекта к камере.
4. Вставьте второй конец USB-кабеля в свободный USB-разъем Raspberry Pi 5.
5. Зафиксируйте подключенный кабель стяжками так, чтобы он не попадал в область вращения пропеллеров.

## Настройка

### Запуск через `clover2-settings`

Чтобы включить тепловизионную камеру в Клевер необходимо открыть настройки:

Запустите узел следующей командой:

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

Проверьте кодировку, шаг строки и частоту:

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

Исходные файлы находятся в папке `clover2/examples/thermal_camera`.

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

1. `subscribe_raw_image.py` подписывается на raw-кадр и публикует строку `/thermal_camera/status`.
2. `visualize_raw_thermal.py` берет верхнюю половину кадра и публикует `/thermal_camera/image_colormap`.
3. `find_temperature_extremes.py` публикует `/thermal_camera/min_temperature`, `/thermal_camera/max_temperature`, `/thermal_camera/center_temperature` и точки `geometry_msgs/msg/PointStamped`:
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

Верхняя половина используется для визуализации и наложения `colormap`. Нижняя половина читается как `uint16` и переводится в градусы Цельсия.

```python
raw = np.frombuffer(msg.data[:msg.height * msg.width * 2], dtype="<u2").reshape(msg.height, msg.width)
temperature_raw = raw[msg.height // 2:, :]
temperature_c = temperature_raw.astype(np.float32) / 64.0 - 273.15
```
