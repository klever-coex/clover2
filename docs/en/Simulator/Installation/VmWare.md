# VMware Workstation

## How to install VMware

1. Navigate to the [official Broadcom catalog](https://support.broadcom.com/group/ecx/productdownloads?freeDownloads=true&subfamily=VMware+Workstation+Pro).
2. Browse the available versions and download the required version of VMware Workstation Pro for Windows.
3. Log in to your existing Broadcom account. If you do not have one, follow the prompts to sign up.
4. Select the `VMware Workstation Pro for Windows` and the latest stable version available.
5. Accept the download terms. If prompted by the portal, complete your profile details to proceed.
6. Download the `.exe` installer file. Once the download is complete, open the file and follow the wizard instructions.
7. If prompted by the installer, restart your computer to ensure all components are correctly configured.

```{note}
   VMware Workstation Pro is available for personal, educational, and commercial use without requiring a license key.
```
  
For details regarding usage rights, see the [Broadcom documentation on downloading and licensing](https://knowledge.broadcom.com/external/article/368667/download-and-license-information-for-vmw.html).

## How to Import a Virtual Machine from an OVA File

1. Launch VMware Workstation Pro.
2. Navigate to `File` → `Open`, select `Open a Virtual Machine` or use the shortcut `Ctrl+O`.

   ```{figure} @assets@/common/simulator/vmware/open-virtual-machine.webp
   :alt: Opening the virtual machine
   :width: 90%
   :align: center
   Opening the virtual machine
   ```

   <br>

3. Select the prepared `.ova` file and click `Open`. Do not extract the OVA file prior to importing.
4. Specify a name for the virtual machine and choose a destination folder for its files. Ensure the target drive has sufficient free disk space for the virtual disks.
5. Click `Import`. If prompted, accept the license agreement and wait for the process to reach completion.

If an error occurs regarding incompatible OVA specifications or virtual hardware, click `Retry`. If prompted to attempt the import using a less strict compatibility check, confirm the action.

## How to Configure the Virtual Machine

1. Select the imported virtual machine from the library and click `Edit virtual machine settings`.

   ```{figure} @assets@/common/simulator/vmware/virtual-machine-settings.webp
   :alt: Virtual machine settings
   :width: 90%
   :align: center
   Virtual machine settings
   ```

   <br>

2. Navigate to `Processors` → `Virtualization Engine` and uncheck the box for `Virtualize Intel VT-x/EPT or AMD-V/RVI`.

   ```{figure} @assets@/common/simulator/vmware/processor-virtualization.webp
   :alt: Unchecking Virtualize Intel VT-x/EPT or AMD-V/RVI
   :width: 90%
   :align: center
   Unchecking Virtualize Intel VT-x/EPT or AMD-V/RVI
   ```

   <br>

3. Navigate to `Display` section and increase the graphics memory to 1 GB or more.

   ```{figure} @assets@/common/simulator/vmware/display-acceleration.webp
   :alt: Increasing memory
   :width: 90%
   :align: center
   Increasing memory
   ```

   <br>

## How to Start the Virtual Machine

1. Click `Start up this guest operating system` to initiate the virtual machine.

   ```{figure} @assets@/common/simulator/vmware/start-virtual-machine.webp
   :alt: Virtual machine start
   :width: 90%
   :align: center
   Virtual machine start
   ```

   <br>

2. Wait for the Linux operating system to boot, then log in using the credentials provided in the image description.
3. Follow the procedures outlined in the {ref}`Simulator Launch <simulator-launch>`.
