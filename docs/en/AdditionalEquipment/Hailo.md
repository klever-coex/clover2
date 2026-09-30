# Hailo-8L (AI HAT+) Neural Accelerator

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Hailo/HailoModel
```

The **Hailo-8L** is a high-performance neural accelerator connected to the Raspberry Pi 5 via PCIe. It offloads neural network computations from the Raspberry Pi 5's main processor, significantly increasing system efficiency.

The Hailo-8L functions as a co-processor. The Raspberry Pi 5 receives an image from the camera, prepares the data, and processes the result, while the Hailo-8L performs the bulk of the neural network computations.

Hailo utilizes the **Hailo Executable Format** (`.hef`). Standard PyTorch weight files (`.pt`) or ONNX models (`.onnx`) must first be compiled into a HEF file for the target Hailo architecture. For detailed instructions, refer to the section  [How to Convert YOLOv8 to HEF](Hailo/HailoModel.md).

## Principle of Operation

The workflow for running a model consists of three main components:

```text
Image -> User Program -> Hailo-8L -> Neural Network Result
```

- **PCIe Driver** connects the Hailo-8L to the operating system.
- **HailoRT** loads the HEF file, configures the accelerator, and executes the model (inference).
- **User Program** captures the image, prepares it for the model, and processes the final output.

Inference is the process of running a trained neural network on input data. For a YOLO-based application, a program typically follows these steps:

1.	Receives a frame from the camera.
2.	Resizes the frame.
3.	Converts the color channel order (e.g., to RGB).
4.	Passes the processed frame to the Hailo-8L.
5.	Receives the inference result;
6.	Converts the raw result into actionable object information.
   
```{tip}
Not all models share the same input dimensions, color channel orders, or data formats. Verify these parameters within the HEF description.
```

## How to Install the PCIe Driver

Use a tool like FileZilla or WinSCP to transfer the driver package (hailort-pcie-driver_4.23.0_all.deb) to the root directory of your Raspberry Pi 5.

Install the package and reboot the system:

```bash
cd ~
sudo apt update
sudo apt install ./hailort-pcie-driver_4.23.0_all.deb
sudo reboot
```

After rebooting, verify that the accelerator is detected:

```bash
lspci | grep -i hailo
ls -l /dev/hailo0
```

Expected Result: The `lspci` command should return the Hailo device.

```text
Co-processor: Hailo Technologies Ltd. Hailo-8 AI Processor
```

## How to Install HailoRT

HailoRT is the runtime library that enables your software to interact with the Hailo-8L. It provides the necessary tools to load HEF files, configure the accelerator, and manage inference. 

Install the required dependencies:

```bash
sudo apt update
sudo apt install -y git cmake libzmq3-dev
```

Download the HailoRT source code (version 4.23.0):

```bash
cd ~
git clone https://github.com/hailo-ai/hailort.git
cd hailort
git fetch --tags
git checkout v4.23.0
```

Build and install HailoRT:

```bash
cmake . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/usr
sudo cmake --build build --target install -j$(nproc)
```

Verify the installation:

```bash
which hailortcli
hailortcli --version
hailortcli scan
```

* `which hailortcli` should return the installation path, e.g. `/usr/bin/hailortcli`.
* `hailortcli --version` should display the installed HailoRT version.
* `hailortcli scan` checks for available Hailo devices. The connected accelerator should appear in the output.
  
```{caution}
If `hailortcli` is installed but `scan` fails to detect the device, the issue is likely related to the physical connection, the PCIe driver, or version compatibility, rather than the HEF file.
```

## How to Install the Python Module

The `hailo_platform` Python module is required to run models within your own Python applications or ROS 2 nodes.

```bash
cd ~/hailort/hailort/libhailort/bindings/python/platform
HAILORT_INCLUDE_DIR=/usr/include \
LIBHAILORT_PATH=/usr/lib/aarch64-linux-gnu/libhailort.so \
python3 -m pip install --user --break-system-packages .
```

Verify the import:

```bash
python3 -c "from hailo_platform import HEF, VDevice; print('HailoRT Python: OK')"
```

If the command outputs `HailoRT Python: OK`, the module is correctly installed for your current Python interpreter.


## How to Verify the HEF

Create a dedicated directory for your models:

```bash
mkdir -p ~/hailo_models
```

Once you have copied your model to the folder, verify its properties using the following command:

```bash
hailortcli parse-hef ~/hailo_models/viz.hef
```

For a YOLOv8 model with a `640x640` input, the output should confirm:

- The Hailo-8L architecture;
- An input shape of `640x640x3`;
- The presence of model outputs or built-in NMS
  
Run a performance benchmark to ensure the model executes correctly:

```bash
hailortcli run ~/hailo_models/viz.hef
```

```{tip}
If the command reports that the HEF is incompatible with the device, verify the HEF architecture. For example, a HEF compiled specifically for the `hailo8` is not compatible with the `hailo8l` device.
```

## How to Use HEF in Your Own Program

The following minimal Python script demonstrates how to load a HEF file and print information about its inputs and outputs:

```python
from hailo_platform import HEF

hef = HEF("/home/pi/hailo_models/viz.hef")

print(hef.get_input_vstream_infos())
print(hef.get_output_vstream_infos())
```

This script only retrieves the model's metadata. To perform actual image processing, your program must:

1. Capture a frame from the camera.
2. Preprocess the frame to match the HEF input requirements.
3. Pass the frame to the Hailo accelerator using `InferVStreams`.
4. Post-process the model output into usable data.

In ROS 2, this workflow is typically implemented within a dedicated node. The node subscribes to a `sensor_msgs/msg/Image` topic, performs preprocessing and inference, and then publishes the results, e.g. as `vision_msgs/msg/Detection2DArray`.

## Troubleshooting

| Issue | What to Check |
|---|---|
| No `/dev/hailo0` device found | AI HAT+ connection, PCIe driver installation, `dkms status` |
| `hailortcli scan` fails to detect device | Physical connectivity, PCIe driver installation, `hailo_pci` module loaded |
| `hailo_platform` cannot be imported | Python module installation, correct Python interpreter, path to `libhailort.so` |
| HEF fails to run | HEF architecture, compatibility with `hailo8l` |
| YOLO runs but detects poorly | Input image dimensions, color channel order (RGB vs. BGR), data types, model parameters, calibration dataset |
