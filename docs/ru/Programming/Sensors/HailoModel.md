# Перевод YOLOv8 в HEF

Для запуска своей модели на Hailo-8L необходимо преобразовать ONNX в формат HEF. Компиляция выполняется на компьютере с 64-битной Ubuntu, а готовый HEF копируется на Raspberry Pi.

В этой статье в качестве примера используется WSL2 с Ubuntu 24.04. Вместо WSL2 можно использовать обычный компьютер или виртуальную машину с Ubuntu Desktop 24.04. Команды установки и компиляции при этом остаются такими же.

```text
best.pt -> model.onnx -> model.har -> model.hef
```

В примере используется модель YOLOv8 с входом `640x640` и три класса.

## Установка компилятора

Файл Hailo Dataflow Compiler должен находиться в домашней директории Ubuntu:

```text
~/hailo_dataflow_compiler-3.34.0-py3-none-linux_x86_64.whl
```

Для компилятора используется Python 3.10. Создайте рабочую директорию и виртуальное окружение в WSL2 или Ubuntu Desktop:

```bash
mkdir -p ~/hailo_work
cd ~/hailo_work

python3.10 -m venv heilo_venv
source heilo_venv/bin/activate
python -m pip install --upgrade pip
pip install ~/hailo_dataflow_compiler-3.34.0-py3-none-linux_x86_64.whl
```

Проверьте установку:

```bash
hailo --version
```

## Подготовка ONNX

Переместите ONNX в рабочую директорию:

```bash
cp best.onnx ~/hailo_work/viz.onnx
cd ~/hailo_work
source heilo_venv/bin/activate
```

Модель в этом примере должна иметь один вход `1x3x640x640` и голову детекции YOLOv8.

## Подготовка изображений для калибровки

Во время компиляции модель переводится в INT8. Для этого нужны изображения, похожие на реальные кадры камеры.

Поместите изображения JPG или PNG в папку:

```text
~/hailo_work/calib_images
```

Калибровочный набор должен содержать разные объекты, расстояния, фоны и условия освещения. Для хорошего результата рекомендуется использовать не менее 512 изображений.

Создайте файл `prepare_calib.py`:

```python
from pathlib import Path

import cv2
import numpy as np


images = []

for path in sorted(Path("calib_images").glob("*")):
    frame = cv2.imread(str(path))
    if frame is None:
        continue

    frame = cv2.resize(frame, (640, 640))
    frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    images.append(frame)

if not images:
    raise RuntimeError("Калибровочные изображения не найдены")

dataset = np.asarray(images, dtype=np.uint8)
np.save("viz_calib_640.npy", dataset)
print(dataset.shape, dataset.dtype)
```

Запустите подготовку:

```bash
python prepare_calib.py
```

Ожидаемый формат результата:

```text
(количество_изображений, 640, 640, 3) uint8
```

## Настройка YOLOv8

Создайте файл `viz_nms_config.json`:

```json
{
  "nms_scores_th": 0.2,
  "nms_iou_th": 0.7,
  "image_dims": [640, 640],
  "max_proposals_per_class": 100,
  "classes": 3,
  "regression_length": 16,
  "background_removal": false,
  "bbox_decoders": [
    {
      "name": "viz/bbox_decoder41",
      "stride": 8,
      "reg_layer": "viz/conv41",
      "cls_layer": "viz/conv42"
    },
    {
      "name": "viz/bbox_decoder52",
      "stride": 16,
      "reg_layer": "viz/conv52",
      "cls_layer": "viz/conv53"
    },
    {
      "name": "viz/bbox_decoder62",
      "stride": 32,
      "reg_layer": "viz/conv62",
      "cls_layer": "viz/conv63"
    }
  ]
}
```

Параметр `classes` должен совпадать с количеством классов модели.

Создайте файл `viz.alls`:

```text
normalization1 = normalization([0.0, 0.0, 0.0], [255.0, 255.0, 255.0])
change_output_activation(conv42, sigmoid)
change_output_activation(conv53, sigmoid)
change_output_activation(conv63, sigmoid)
nms_postprocess("viz_nms_config.json", meta_arch=yolov8, engine=cpu)

allocator_param(width_splitter_defuse=disabled)
```

Эта настройка добавляет нормализацию входа и обработку результатов YOLOv8.

## Компиляция модели

Компиляция состоит из трёх шагов.

### 1. Парсинг ONNX

```bash
hailo parser onnx viz.onnx \
  --net-name viz \
  --har-path viz_heads.har \
  --hw-arch hailo8l \
  -y \
  --end-node-names \
  /model.22/cv2.0/cv2.0.2/Conv \
  /model.22/cv3.0/cv3.0.2/Conv \
  /model.22/cv2.1/cv2.1.2/Conv \
  /model.22/cv3.1/cv3.1.2/Conv \
  /model.22/cv2.2/cv2.2.2/Conv \
  /model.22/cv3.2/cv3.2.2/Conv
```

Имена конечных узлов подходят для модели YOLOv8 из примера. У другой версии YOLO они могут отличаться.

### 2. Оптимизация

```bash
hailo optimize viz_heads.har \
  --hw-arch hailo8l \
  --calib-set-path viz_calib_640.npy \
  --model-script viz.alls \
  --output-har-path viz_optimized.har
```

### 3. Создание HEF

```bash
LD_LIBRARY_PATH="$PWD/heilo_venv/lib/python3.10/site-packages/hailo_tools/or-tools/dependencies/install/lib" \
hailo compiler viz_optimized.har \
  --hw-arch hailo8l \
  --output-dir "$PWD"
```

После успешной компиляции в `~/hailo_work` появится файл:

```text
viz.hef
```

## Размещение модели на Raspberry Pi

С помощью FileZilla или WinSCP перенесите `viz.hef` в домашнюю директорию Raspberry Pi. Затем подключитесь к дрону, создайте директорию моделей и переместите HEF:

```bash
mkdir -p ~/hailo_models
mv ~/viz.hef ~/hailo_models/
```

Проверьте модель на Raspberry Pi:

```bash
hailortcli parse-hef ~/hailo_models/viz.hef
hailortcli run ~/hailo_models/viz.hef
```

## Что важно проверить

- HEF должен быть скомпилирован с `--hw-arch hailo8l`.
- Размер входа ONNX, калибровочных изображений и входа программы должен совпадать.
- Изображение должно передаваться в RGB, а не в BGR.
- Порядок имён классов должен совпадать с порядком классов при обучении.
