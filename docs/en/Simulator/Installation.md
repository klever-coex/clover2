# Simulator Installation Guide

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

Installation/DevContainer
Installation/VmWare
Installation/UTM
```

To begin working with the simulator, first set up a compatible software environment. One effective method is to run Linux within a virtual machine (VM) hosted on your existing operating system, such as Windows or macOS. This allows you to utilize a full Linux environment without the need to replace or dual-boot your current operating system.

```{important}
While Linux users can technically use VMware Workstation Pro with the provided `.ova` image, this configuration has not been officially tested. For the most stable experience on Linux, we strongly recommend using the [Dev Container](./Installation/DevContainer).
```

## Choosing an Installation Method

Select an installation path based on your computer's operating system and processor architecture:

* **Linux**: use [Dev Container](./Installation/DevContainer). This provides an isolated development environment running within Docker, allowing you to access all necessary tools without the overhead of a separate virtual machine.
* **Windows** (Intel or AMD (x86_64): use [VMware Workstation Pro](./Installation/VmWare) with the pre-configured image in `.ova` format.
* **Mac** (Apple Silicon/ARM64, M-series): use [UTM](./Installation/UTM) with the pre-configured image in `.qcow2` format.

```{hint}
If you are unsure which processor your computer uses, you can verify the model in your system settings
```

## Downloading the Image

Before starting the installation, download the pre-configured image that matches your chosen method:

[TODO: Insert link to the latest release]

Verify the file format before downloading:
*	**VMware Workstation Pro** requires a `.ova` image.
*	**UTM** requires a `.qcow2` image.
*	**Dev Container**. Follow the specific environment setup instructions; a standalone virtual machine image may not be necessary.

Once your environment is prepared, proceed to the {doc}`Usage Guide <Usage>` section for instructions on the commands required to build and launch the simulator.
