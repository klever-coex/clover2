# clover2_bringup

Пакет содержит все launch-файлы, необходимые для запуска проекта, а также кастомные actions для автоматизации конфигурации.

## Структура

```text
launch/
├── klever5.launch.xml       # главный launch-файл для полного запуска Клевер 5
├── description.launch.xml   # публикация TF (URDF)
├── sensing/                 # драйверы устройств
│   ├── camera.launch.xml
│   ├── 2d_lidar.launch.xml
│   └── thermal_camera.launch.xml
├── vision/                  # обработка изображения
│   ├── camera_pipeline.launch.xml
│   ├── feature_detector.launch.xml
│   └── optical_flow.launch.xml
└── snippets/                # переиспользуемые фрагменты
    ├── component_container.launch.xml
    ├── fcu_bridge.launch.xml
    ├── localization.launch.xml
    ├── indication.launch.xml
    └── display.launch.xml
```

## Главный launch-файл `klever5.launch.xml`

Собирает всё вместе: описание (TF), камеры с пайплайном обработки, мост к полётному контроллеру, локализацию, индикацию и диагностику.

Основные конфигурационные аргументы:

| Аргумент                                | По умолчанию | Описание                                            |
| --------------------------------------- | ------------ | --------------------------------------------------- |
| `main_camera.enable`                    | `true`       | Запуск основной камеры и её пайплайна               |
| `main_camera.feature_detector`          | `true`       | Запуск детектора признаков                          |
| `front_camera.enable`                   | `false`      | Запуск фронтальной камеры                           |
| `optical_flow.enable`                   | `true`       | Запуск оптического потока                           |
| `fcu_bridge.fcu_conn`                   | `uart`       | Тип соединения с FCU                                |
| `localization.aruco_tracker.enable`     | `true`       | Запуск трекера ArUco-маркеров                       |
| `localization.map_server.enable`        | `true`       | Запуск сервера карты                                |
| `localization.map_server.map_filename`  | `example-1.yaml` | Файл карты                                      |
| `indication.led_strip`                  | `false`      | Запуск LED-ленты                                    |
| `indication.display`                    | `false`      | Запуск дисплея                                      |
| `additional_sensors.2d_lidar`           | `false`      | Запуск 2D-лидара                                    |
| `additional_sensors.thermal_camera`     | `false`      | Запуск тепловой камеры                              |
| `sensing_pkg`                           | `clover2_bringup` | Пакет, из которого загружаются sensing launch-файлы |
| `root_ns`                               | —            | Корневой namespace (пустой или начинается с `/`)    |
| `params_file`                           | `params/klever5.yaml` | Файл дополнительных параметров              |

Пример:

```
ros2 launch clover2_bringup klever5.launch.xml additional_sensors.2d_lidar:=true indication.display:=true
```

## Группа запуска сенсоров (sensing)

Для стандартизации способа запуска драйверы устройств вынесены в отдельные файлы в папку `launch/sensing`. Launch-файлы имеют набор стандартных аргументов.

### Шаблон аргументов запуска драйвера камеры

| Аргумент        | Обязательный | По умолчанию          | Описание                                                                                       |
| --------------- | ------------ | --------------------- | ---------------------------------------------------------------------------------------------- |
| `camera_id`     | Да           |                       | Идентификатор камеры (числовой для libcamera или путь dts)                                      |
| `camera_name`   | Нет          | `camera`              | Имя ноды камеры. Также выбирает файл `params/libcamera/$(camera_name).yaml`                     |
| `container_name`| Нет          | `camera_container`    | Имя ROS 2 контейнера для загрузки драйвера камеры как компонента                                 |
| `frame_id`      | Нет          | `$(camera_name)_link` | Идентификатор TF2 vision frame                                                                  |
| `ns`            | Нет          | —                     | ROS 2 namespace камеры                                                                          |
| `params_file`   | Нет          | `params/klever5.yaml` | Путь к файлу дополнительных параметров                                                           |
| `use_composition`| Нет         | `false`               | Если `true` — драйвер камеры загружается как компонент в контейнер `$(container_name)`. Иначе как отдельная нода |

### Шаблон аргументов запуска 2D-лидара

| Аргумент      | Обязательный | По умолчанию           | Описание                                   |
| ------------- | ------------ | ---------------------- | ------------------------------------------ |
| `lidar_id`    | Нет          | `/dev/rplidar`         | Путь к последовательному устройству лидара |
| `lidar_name`  | Нет          | `lidar`                | Имя ноды лидара                            |
| `ns`          | Нет          | —                      | ROS 2 namespace лидара                     |
| `params_file` | Нет          | `params/klever5.yaml`  | Путь к файлу дополнительных параметров     |

### Шаблон аргументов запуска тепловой камеры

| Аргумент                | Обязательный | По умолчанию         | Описание                           |
| ----------------------- | ------------ | -------------------- | ---------------------------------- |
| `thermal_camera_device` | Нет          | `/dev/thermal_camera`| Путь к устройству тепловой камеры  |
| `thermal_camera_name`   | Нет          | `thermal_camera`     | Имя ноды тепловой камеры           |
| `ns`                    | Нет          | —                    | ROS 2 namespace тепловой камеры    |

### Подмена пакета сенсоров

Главный launch-файл `klever5.launch.xml` принимает аргумент `sensing_pkg`, который определяет пакет, из которого загружаются launch-файлы сенсоров. Это позволяет подменить стандартные драйверы камеры, 2D-лидара и тепловой камеры на пользовательские без изменения оригинального пакета.

Пример использования — запуск klever5 с пользовательским пакетом сенсоров:

```
ros2 launch clover2_bringup klever5.launch.xml sensing_pkg:=my_custom_sensing
```

Подставляемый пакет должен предоставлять те же пути к launch-файлам (например, `launch/sensing/2d_lidar.launch.xml`, `launch/sensing/camera.launch.xml`, `launch/sensing/thermal_camera.launch.xml`).
