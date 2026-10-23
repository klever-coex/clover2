# UTM

[UTM](https://mac.getutm.app/) is a free, open-source application designed for creating and managing virtual machines on macOS. 

UTM offers two distinct operating modes:
* **Virtualization** runs a guest operating system on the same processor architecture as your host computer.
* **Emulation** simulates a different processor architecture, allowing you to run operating systems designed for hardware different from your own.

UTM utilizes QEMU technology and is compatible with both Intel and Apple Silicon-based Macs.

:::{attention}
These instructions and the provided ARM64 virtual machine image are specifically designed for Mac computers with Apple Silicon (M-series) processors.
:::

## How to install UTM

Download UTM from the official website or retrieve the latest stable [release from GitHub](https://github.com/utmapp/UTM/releases/latest/download/UTM.dmg). 

To install, open the downloaded `.dmg` file, drag `UTM.app` into your `Applications` folder, and launch the program.

Starting with UTM version 4.7, you can import existing images directly through the setup wizard.

## How to Create a Machine from a QCOW2 Image

For Apple Silicon, ensure you are using a Linux image specifically built for the **ARM64 / aarch64** architecture.

1. Click `File` → `New...` or press the `+` button to begin creating a virtual machine.
2. Select `Virtualize` → `Linux`.
3. Configure parameters on the `Hardware` page:
   - RAM. We recommend allocating at least 4 GB. If your system has sufficient memory, 8 GB or more is preferred.
   - CPU cores. We recommend allocating at least 2 cores. If your system permits, select 4 cores or more.
   - Graphics. Check `Enable hardware OpenGL acceleration` to improve graphical performance within the simulator.

   ```{image} @assets@/common/simulator/utm/hardware.webp
   :alt: Gazebo
   :width: 100%
   ```
4. Adjust the following settings on the Linux page:
   - Ensure the `Use Apple Virtualization` is turned off.
   - In the `Boot Image Type` section, select `Import existing drive`.
   - Click `Browse…` and select your `.qcow2` image file.
5. Click `Continue`. You may skip the shared folder configuration for this step.
6. On the `Summary` summary page, enter a name for your virtual machine and click `Save`. Wait for the disk import process to finish.
7. Select your new machine from the list and click the `▶` button to launch it. Once Linux boots, log in using the credentials provided in the image description.

If the **UEFI Shell** opens instead of Linux, or if you see an error stating no boot device was found, stop the virtual machine and verify these settings:
* Go to `Edit` → `Drives` and confirm that your `.qcow2` image is correctly connected as a disk.
* Ensure the image is intended for the ARM64 and supports UEFI booting.
* Verify that `UEFI boot` is enabled In the `QEMU` section.

Once you have successfully logged into Linux, refer to the {ref}`Simulator Launch <simulator-launch>`.
