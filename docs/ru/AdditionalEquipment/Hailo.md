# Нейронный ускоритель Hailo-8L (AI HAT+)

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Hailo/HailoModel
```

Hailo-8L — это нейронный ускоритель, подключенный к Raspberry Pi 5 по PCIe. Он позволяет запускать нейросети, не нагружая основной процессор Raspberry Pi.

Hailo работает с моделями в формате **Hailo Executable Format** (`.hef`). Обычные веса PyTorch (`.pt`) и модели ONNX (`.onnx`) сначала необходимо скомпилировать в HEF на компьютере. Этот процесс описан в разделе [Перевод YOLOv8 в HEF](Hailo/HailoModel.md).

## Принцип работы

Работа с моделью состоит из трёх частей:

```text
Изображение -> программа пользователя -> Hailo-8L -> результат нейросети
```

- PCIe-драйвер подключает Hailo-8L к операционной системе;
- HailoRT загружает HEF и запускает модель;
- программа пользователя подготавливает изображение и обрабатывает результат.

Для YOLO программа обычно изменяет размер кадра, переводит изображение в RGB, запускает инференс и получает рамки обнаруженных объектов.

## Установка PCIe-драйвера

С помощью FileZilla/WinSCP переместите драйвер (hailort-pcie-driver_4.23.0_all.deb) в корень Raspberry Pi.

Установите пакет и перезагрузите Raspberry Pi:

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

Ожидаемый результат `lspci` содержит устройство Hailo:

```text
Co-processor: Hailo Technologies Ltd. Hailo-8 AI Processor
```

## Установка HailoRT

HailoRT — это библиотека для загрузки и запуска HEF. Установите необходимые пакеты:

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

Команда `which hailortcli` должна вывести `/usr/bin/hailortcli`, а `hailortcli scan` — подключенный ускоритель.

## Установка Python-модуля

Python-модуль `hailo_platform` нужен для запуска модели из своей программы или ROS 2 ноды:

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

## Проверка HEF

Создайте папку для моделей:

```bash
mkdir -p ~/hailo_models
```

После копирования модели проверьте её описание:

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

## Использование в своей программе

Минимальная программа может открыть HEF и вывести информацию о его входах и выходах:

```python
from hailo_platform import HEF

hef = HEF("/home/pi/hailo_models/viz.hef")

print(hef.get_input_vstream_infos())
print(hef.get_output_vstream_infos())
```

Для обработки изображения программа должна дополнительно:

1. Получить кадр с камеры.
2. Подготовить кадр в том же формате, который использовался при обучении и компиляции модели.
3. Передать кадр в Hailo через `InferVStreams`.
4. Обработать выход модели.

В ROS 2 эти действия можно оформить в отдельную ноду: подписаться на `sensor_msgs/msg/Image` и публиковать результат как `vision_msgs/msg/Detection2DArray`.

## Возможные проблемы

| Проблема | Что проверить |
|---|---|
| Нет `/dev/hailo0` | Установку PCIe-драйвера и вывод `dkms status`. |
| `hailortcli scan` не видит устройство | Подключение Hailo и загрузку модуля `hailo_pci`. |
| Не импортируется `hailo_platform` | Установку Python-модуля и используемый Python. |
| HEF не запускается | Что модель скомпилирована под `hailo8l`. |
| YOLO работает, но распознаёт плохо | Размер входа, RGB/BGR и калибровочный набор. |
