# How to Convert YOLOv8 to HEF

To run your model on the Hailo-8L, convert your ONNX model into the HEF. The compilation process is performed on a 64-bit Ubuntu system, after which the resulting `.hef` file is transferred to your Raspberry Pi 5.

This guide uses WSL2 with Ubuntu 24.04 as an example. You can also follow these exact steps using a native Ubuntu Desktop installation or a virtual machine; the commands remain identical.

The conversion follows this pipeline:

```text
best.pt -> model.onnx -> model.har -> model.hef
```

File Definitions:
* `.pt`: Original trained YOLOv8 model weights.
* `.onnx`: Model exported in ONNX format.
* `.har`: Intermediate Hailo Archive format used during model optimization.
* `.hef`: Final compiled file ready for the Hailo AI accelerator

This example assumes a YOLOv8 model with a `640x640` input size and three output classes.

## How to Install the Compiler

First, ensure your Hailo Dataflow Compiler is located in your Ubuntu home directory: 

```text
~/hailo_dataflow_compiler-3.34.0-py3-none-linux_x86_64.whl
```

The compiler requires Python 3.10. Follow these steps to set up a dedicated working directory and a virtual environment:

```bash
mkdir -p ~/hailo_work
cd ~/hailo_work

python3.10 -m venv heilo_venv
source heilo_venv/bin/activate
python -m pip install --upgrade pip
pip install ~/hailo_dataflow_compiler-3.34.0-py3-none-linux_x86_64.whl
```

In this guide, we use `hailo_venv` as the virtual environment name. Use this name for all subsequent commands.

Verify the installation:

```bash
hailo --version
```

If the command is not found, ensure your environment is active by running: `source ~/hailo_work/hailo_venv/bin/activate`

## How to Prepare the ONNX Model

Move your exported ONNX model into the working directory:

```bash
cp best.onnx ~/hailo_work/viz.onnx
cd ~/hailo_work
source heilo_venv/bin/activate
```

For this process, the model must feature a single input layer of size `1x3x640x640` and a standard YOLOv8 detection head.

## How to Prepare Images for Calibration

During compilation, the model is optimized for INT8 precision. To ensure accuracy, the compiler requires a calibration dataset consisting of images that reflect real-world operating conditions.

Place your JPG or PNG images into the following folder:

```text
~/hailo_work/calib_images
```

The calibration set should include a diverse range of objects, distances, backgrounds, and lighting conditions. For best results, use images similar to those captured by your quadcopter's camera. We recommend using at least 512 images.

Create a file named `prepare_calib.py`:

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
    raise RuntimeError("No calibration images found")

dataset = np.asarray(images, dtype=np.uint8)
np.save("viz_calib_640.npy", dataset)
print(dataset.shape, dataset.dtype)
```

*Run the preparation:

```bash
python prepare_calib.py
```

Expected Output Format:

```text
(number_of_images, 640, 640, 3) uint8
```

```{attention}
`cv2.imread()` loads images in BGR format by default.
The script includes a conversion step to RGB.
```

## How to Configure YOLOv8

Create a file named `viz_nms_config.json`:

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

* `classes` parameter must match the exact number of classes used during your model training.
* `image_dims` parameter must match the input resolution of your model.

Create a file named `viz.alls`:

```text
normalization1 = normalization([0.0, 0.0, 0.0], [255.0, 255.0, 255.0])
change_output_activation(conv42, sigmoid)
change_output_activation(conv53, sigmoid)
change_output_activation(conv63, sigmoid)
nms_postprocess("viz_nms_config.json", meta_arch=yolov8, engine=cpu)

allocator_param(width_splitter_defuse=disabled)
```

This configuration includes input image normalization and YOLOv8 result processing. 
Normalization converts the input image channel values from the `0–255` range to `0–1` range.

## How to Compile the Model

The compilation process consists of three distinct stages:

### 1. Parsing the ONNX Model

In this stage, the Hailo Dataflow Compiler converts the `.onnx` model into the internal `.har` format.

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

The end node names provided in the example are specifically configured for the YOLOv8 model used in this guide. If you are using a different version of YOLO or a custom model architecture, these names will likely differ.

```{caution}
If the compiler reports that the specified end node was not found, you must verify the node names within your specific ONNX model. The end nodes must correspond exactly to the output layers you intend to use for detection.
```

### 2. Optimization

In this stage, the model is optimized for execution on the Hailo-8L.

```bash
hailo optimize viz_heads.har \
  --hw-arch hailo8l \
  --calib-set-path viz_calib_640.npy \
  --model-script viz.alls \
  --output-har-path viz_optimized.har
```

### 3. Creating the HEF

In the final stage, the optimized `.har` model is compiled into a `.hef` file, which is ready to be loaded onto the Hailo-8L.

Run the compilation command:

```bash
LD_LIBRARY_PATH="$PWD/heilo_venv/lib/python3.10/site-packages/hailo_tools/or-tools/dependencies/install/lib" \
hailo compiler viz_optimized.har \
  --hw-arch hailo8l \
  --output-dir "$PWD"
```

Upon successful completion, the following file will be generated in `~/hailo_work` directory:

```text
viz.hef
```

## How to Deploy the Model to the Raspberry Pi 5

Use FileZilla or WinSCP to transfer `viz.hef` to the home directory of your Raspberry Pi 5.
Connect to your Raspberry Pi 5 via terminal and run the following commands to create a dedicated models directory and move the file:

```bash
mkdir -p ~/hailo_models
mv ~/viz.hef ~/hailo_models/
```

Use the HailoRT tools to ensure the model is functional:

```bash
hailortcli parse-hef ~/hailo_models/viz.hef
hailortcli run ~/hailo_models/viz.hef
```

* `parse-hef` displays metadata about the HEF and verifies that the file is valid and readable by HailoRT.
* `run` executes the model through HailoRT.

## Critical Checklist for Troubleshooting

To avoid compilation or runtime errors, verify the following:

- The HEF must be compiled using `--hw-arch hailo8l`.
- The ONNX input size, calibration image size, and application input must all match.
- Input images must be passed in RGB format, not BGR.
- The order of class names in your configuration must match the order used during training.
- The end node names in your parser configuration must match your ONNX model exactly. 
- The `bbox_decoders`parameters must correspond to the specific output structure of your YOLOv8 version. 
