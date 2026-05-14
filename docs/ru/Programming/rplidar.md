# RPLidar C1

```{contents}
:depth: 2
:local:
:class: custom-toc
```

<br>

```{figure} ../../assets/rplidar/Lidar.webp
:alt: RPLidar C1
:width: 500px
:align: center

Рисунок 1 — RPLidar C1
```

RPLIDAR C1 — это 2D-лидар для кругового сканирования окружающей области (см. рисунок 1). Он последовательно измеряет расстояния в разных направлениях и формирует набор точек вокруг датчика в диапазоне 360 градусов.

## Принцип работы

Лидар испускает короткий инфракрасный лазерный сигнал в одном направлении. Луч отражается от ближайшего объекта и возвращается обратно в датчик. По отражённому сигналу электроника лидара вычисляет расстояние до объекта.

Одного измерения недостаточно, чтобы получить картину вокруг дрона. Поэтому измерительный модуль, похожий на небольшой лазерный дальномер, постоянно вращается и делает много таких замеров под разными углами. В результате один полный оборот превращается в набор точек: для каждой точки известны угол и расстояние.

Пример одного измерения:

```text
угол:      45°
расстояние: 1.8 м
```

Если рядом нет объекта или отражённый сигнал слишком слабый, измерение может быть невалидным. Такие значения обычно игнорируются алгоритмами навигации и построения карты.

```{figure} ../../assets/rplidar/coord_scheme.webp
:alt: Определение системы координат данных сканирования RPLIDAR C1
:width: 500px
:align: center

Рисунок 2 — Определение системы координат данных сканирования RPLIDAR C1
```

## Механическая система

Сканирующий модуль RPLidar C1 вращается по часовой стрелке. Обычная частота сканирования — 10 Гц, то есть лидар делает около 10 полных круговых сканов в секунду.

## Система координат лидара

Данные лидара удобно представить в полярной системе координат (см. рисунок 2):

- начало координат находится в центре лидара;
- направление задаётся углом;
- дальность задаётся расстоянием от центра лидара до препятствия.

Ось X направлена вперёд по направлению «мёртвой зоны» датчика. Угол отсчитывается по часовой стрелке и увеличивается по мере вращения сканирующего модуля.

## Данные в ROS 2

В ROS 2 данные лидара публикуются в виде сообщения `sensor_msgs/msg/LaserScan`, обычно в топик `/scan`. Это сообщение содержит параметры одного кругового скана и массив расстояний.

Основные поля `LaserScan`:

| Поле | Описание |
| --- | --- |
| `header.frame_id` | Имя системы координат лидара, например `laser`. |
| `angle_min`, `angle_max` | Начало и конец углового диапазона сканирования. |
| `angle_increment` | Угловой шаг между соседними измерениями. |
| `range_min`, `range_max` | Минимальная и максимальная допустимая дальность. |
| `ranges` | Массив расстояний в метрах. |

Каждый элемент массива `ranges` соответствует одному направлению луча:

```text
angle = angle_min + i * angle_increment
distance = ranges[i]
```

При частоте 10 Гц RPLidar C1 обновляет такой массив примерно 10 раз в секунду. Эти данные можно использовать для обнаружения препятствий, навигации и построения карты.

## Сборка и установка RPLidar C1

1. Установите RPLidar C1 на монтажную деку из поликарбоната при помощи винтов M2.5x6 (см. рисунок 3).

   ```{figure} ../../assets/rplidar/assemble_lidar1.webp
   :alt: Установка RPLIDAR C1
   :width: 700px
   :align: center

   Рисунок 3 — Установка RPLIDAR C1
   ```

2. Установите алюминиевые стойки 40мм на монтажную деку из поликарбоната при помощи винтов М3х6 (см. рисунок 4).

   ```{figure} ../../assets/rplidar/assemble_lidar2.webp
   :alt: Установка алюминиевых стоек
   :width: 700px
   :align: center

   Рисунок 4 — Установка алюминиевых стоек
   ```

3. Установите подготовленный RPLidar на алюминиевые стойки с помощью винтов М3х6 (см. рисунок 5).

   ```{figure} ../../assets/rplidar/assemble_lidar3.webp
   :alt: Установка RPLIDAR C1 на алюминиевые стойки
   :width: 700px
   :align: center

   Рисунок 5 — Установка RPLIDAR C1 на алюминиевые стойки
   ```


4. Подключите RPLidar к Raspberry Pi 5 через порт USB с помощью комплектного провода с переходником.

5. Зафиксируйте подключенный провод с помощью стяжек таким образом, чтобы он не попадал в область вращения пропеллеров.

## Использование

Запуск выполняется из окружения ROS 2.

В первом терминале запустите драйвер RPLidar C1:

```bash
source /opt/ros/jazzy/setup.bash
source ~/clover2_ws/install/setup.bash

ros2 run rplidar_ros rplidar_composition --ros-args \
  -p serial_port:=/dev/rplidar \
  -p serial_baudrate:=460800 \
  -p frame_id:=laser \
  -p inverted:=false \
  -p angle_compensate:=true \
  -p scan_mode:=Standard \
  -p topic_name:=scan
```

При успешном запуске в терминале появится лог:

```text
RPLidar running on ROS2 package rplidar_ros. RPLIDAR SDK Version:2.1.0
RPLidar health status : OK.
current scan mode: Standard, sample rate: 5 Khz, max_distance: 16.0 m, scan frequency:10.0 Hz
```

После успешного запуска лидар начнёт вращаться, а драйвер будет публиковать данные в топик `/scan` с типом сообщения `sensor_msgs/msg/LaserScan`.

Во втором терминале проверьте, что топик появился:

```bash
source /opt/ros/jazzy/setup.bash
source ~/clover2_ws/install/setup.bash

ros2 topic list | grep scan
ros2 topic info /scan
```

Ожидаемый тип топика:

```text
Type: sensor_msgs/msg/LaserScan
```

Проверьте частоту публикации:

```bash
ros2 topic hz /scan
```

Для RPLidar C1 в режиме `Standard` нормальная частота сканирования составляет 10 Гц.

Для просмотра одного сообщения:

```bash
ros2 topic echo --once /scan
```

Если нужно проверить только массив расстояний:

```bash
ros2 topic echo --once /scan --field ranges
```

Если `/scan` публикуется с ненулевой частотой, а поле `ranges` содержит массив расстояний в метрах, лидар работает корректно.

## Приложение: установка драйвера

Драйвер RPLidar для ROS 2 находится в репозитории [`Slamtec/rplidar_ros`](https://github.com/Slamtec/rplidar_ros/tree/ros2). Если пакет `rplidar_ros` отсутствует, установите ROS 2-версию из ветки `ros2`:

```bash
cd ~/clover2_ws/src/clover2/third_party
git clone -b ros2 https://github.com/Slamtec/rplidar_ros.git

cd ~/clover2_ws
source /opt/ros/jazzy/setup.bash
colcon build --symlink-install --packages-select rplidar_ros
source install/setup.bash
```

Во время сборки могут появиться предупреждения из файлов SDK, например `unused parameter` или `ISO C++ forbids zero-size array`. Это не ошибка, если в конце сборки есть строка:

```text
Finished <<< rplidar_ros
```

После установки проверьте, что устройство и пакет доступны:

```bash
ls -l /dev/rplidar

source /opt/ros/jazzy/setup.bash
source ~/clover2_ws/install/setup.bash

ros2 pkg prefix rplidar_ros
ros2 pkg executables rplidar_ros
```
