# Нейронный ускоритель Hailo-8L (AI HAT+)

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Hailo/HailoModel
```

Hailo-8L — нейронный ускоритель, подключенный к Raspberry Pi 5 по PCIe. 
Он позволяет запускать нейросети, не нагружая основной процессор Raspberry Pi 5.

Hailo-8L — не отдельный компьютер: Raspberry Pi 5 получает изображение с камеры, подготавливает данные и обрабатывает результат, а Hailo-8L выполняет основную часть вычислений самой нейронной сети.

Hailo работает с моделями в формате **Hailo Executable Format** (`.hef`). Файлы весов PyTorch (`.pt`) и модели ONNX (`.onnx`) сначала необходимо скомпилировать в HEF на компьютере для нужной архитектуры Hailo. Этот процесс описан в разделе [Перевод YOLOv8 в HEF](Hailo/HailoModel.md).

## Принцип работы

Работа с моделью состоит из трех частей:

```text
Изображение -> программа пользователя -> Hailo-8L -> результат нейросети
```

- PCIe-драйвер подключает Hailo-8L к операционной системе;
- HailoRT загружает HEF, настраивает ускоритель и запускает модель (инференс);
- программа пользователя получает изображение, подготавливает его для модели и обрабатывает результат.

Инференс — это запуск обученной нейронной сети на входных данных. Для YOLO программа обычно выполняет следующие действия:
1.	получает кадр с камеры;
2.	изменяет его размер;
3.	приводит данные к порядку цветовых каналов (RGB);
4.	передает кадр в Hailo-8L;
5.	получает результат инференса;
6.	преобразует результат в необходимую информацию об объектах.
   
```{tip}
Не все модели используют одинаковый размер изображения, порядок цветовых каналов или формат данных. Эти параметры необходимо проверять в описании HEF.
```

## Установка PCIe-драйвера

С помощью FileZilla или WinSCP переместите пакет драйвера (hailort-pcie-driver_4.23.0_all.deb) в корень Raspberry Pi 5.

Установите пакет и перезагрузите Raspberry Pi 5:

```bash
cd ~
sudo apt update
sudo apt install ./hailort-pcie-driver_4.23.0_all.deb
sudo reboot
```

После перезагрузки проверьте, что ускоритель обнаружен:

```bash
lspci | grep -i hailo
ls -l /dev/hailo0
```

Ожидаемый результат: `lspci` содержит устройство Hailo:

```text
Co-processor: Hailo Technologies Ltd. Hailo-8 AI Processor
```

## Установка HailoRT

HailoRT — библиотека для взаимодействия программы с Hailo-8L. Она предоставляет инструменты для загрузки HEF, настройки ускорителя и запуска инференса. 

Установите необходимые пакеты:

```bash
sudo apt update
sudo apt install -y git cmake libzmq3-dev
```

Загрузите исходный код HailoRT версии 4.23.0:

```bash
cd ~
git clone https://github.com/hailo-ai/hailort.git
cd hailort
git fetch --tags
git checkout v4.23.0
```

Соберите и установите HailoRT:

```bash
cmake . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr
sudo cmake --build build --target install -j$(nproc)
```

Проверьте установку:

```bash
which hailortcli
hailortcli --version
hailortcli scan
```

* Команда `which hailortcli` должна вывести путь к установленной программе, например `/usr/bin/hailortcli`.
* Команда `hailortcli --version` должна показать установленную версию HailoRT.
* Команда `hailortcli scan` проверяет доступные устройства Hailo. В результате должен отображаться подключенный ускоритель.

```{caution}
Если `hailortcli` установлена, но `scan` не видит устройство, проблема обычно связана не с HEF, а с подключением устройства, PCIe-драйвером или совместимостью версий.
```

## Установка Python-модуля

Python-модуль `hailo_platform` нужен для запуска модели из своей Python-программы или ROS 2-ноды.

```bash
cd ~/hailort/hailort/libhailort/bindings/python/platform
HAILORT_INCLUDE_DIR=/usr/include \
LIBHAILORT_PATH=/usr/lib/aarch64-linux-gnu/libhailort.so \
python3 -m pip install --user --break-system-packages .
```

Проверьте импорт:

```bash
python3 -c "from hailo_platform import HEF, VDevice; print('HailoRT Python: OK')"
```

Если команда выводит `HailoRT Python: OK`, Python-модуль доступен для текущего интерпретатора Python.


## Проверка HEF

Создайте папку для моделей:

```bash
mkdir -p ~/hailo_models
```

После копирования модели проверьте ее описание:

```bash
hailortcli parse-hef ~/hailo_models/viz.hef
```

Для модели YOLOv8 с входом `640x640` в выводе должны присутствовать:

- архитектура Hailo-8L;
- вход размером `640x640x3`;
- выход модели или встроенного NMS.

Запустите проверку производительности модели:

```bash
hailortcli run ~/hailo_models/viz.hef
```

```{tip}
Если команда сообщает, что HEF несовместима с устройством, проверьте архитектуру HEF.
Например, HEF, скомпилированная для `hailo8`, не подходит для устройства `hailo8l`.
```

## Использование в своей программе

Минимальная программа может открыть HEF и вывести информацию о ее входах и выходах:

```python
from hailo_platform import HEF

hef = HEF("/home/pi/hailo_models/viz.hef")

print(hef.get_input_vstream_infos())
print(hef.get_output_vstream_infos())
```

В этом примере программа пока не запускает нейронную сеть. 
Она только открывает `HEF` и получает описание входных и выходных потоков модели.
Для обработки изображения программа должна дополнительно:

1. Получить кадр с камеры.
2. Подготовить кадр в соответствии с входным форматом `HEF`.
3. Передать кадр в Hailo через `InferVStreams`.
4. Обработать выход модели.

В ROS 2 эти действия можно оформить в отдельную ноду: подписаться на `sensor_msgs/msg/Image`, подготовить изображение, запустить инференс и опубликовать результат, например, как  `vision_msgs/msg/Detection2DArray`.

## Возможные проблемы

| Проблема | Что проверить |
|---|---|
| Нет `/dev/hailo0` | Подключение AI HAT+, установку PCIe-драйвера и вывод `dkms status` |
| `hailortcli scan` не видит устройство | Подключение Hailo, PCIe-драйвер и загрузку модуля `hailo_pci` |
| Не импортируется `hailo_platform` | Установку Python-модуля, используемый Python и путь к `libhailort.so` |
| HEF не запускается | Архитектуру HEF и совместимость с `hailo8l` |
| YOLO работает, но распознает плохо | Размер входного изображения, RGB/BGR, тип данных, параметры и калибровочный набор |
