# Firmware Guide

```{note}
This guide describes how to flash the PX4 firmware to Betaflight on a **MicoAir743v2** flight controller using DFU mode.
```

```{caution}
This guide applies only to the **MicoAir743v2** flight controller.
A full memory clearance will delete the installed PX4 firmware and its settings.
Before you proceed, make sure you have selected the correct controller and firmware file.
```

```{figure} @assets@/ru/assembly/clover5-fpv/firmware/micoair743v2.webp
:alt: Полётный контроллер MicoAir743v2
:width: 350px
:align: center
MicoAir743v2 flight controller
```

<br>

## Required Software and Files

Prepare the following before proceeding:

- [Betaflight App](https://app.betaflight.com/) — the app used to build firmware and configure your flight controller.
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) — utility required to clear the controller memory and flash new firmware.
- Betaflight firmware for **MicoAir743v2** in `.hex` format — build and save it as described below.

Ensure **STM32CubeProgrammer** is fully installed before flashing the device.

## How to Build and Save the HEX File

1. Open the [Betaflight App](https://app.betaflight.com/) and navigate to `Firmware Flasher` tab.

    ```{figure} @assets@/en/assembly/clover5-fpv/firmware/build-open-flasher.webp
    :alt: Opening the Betaflight firmware flasher
    :width: 700px
    :align: center
    Opening the Betaflight firmware flasher
    ```

    <br>

2. In the `Board & Build` section, select the `MICOAIR743V2` board and choose the required firmware release.

    ```{figure} @assets@/en/assembly/clover5-fpv/firmware/build-board-selection.webp
    :alt: Selecting the board and firmware release
    :width: 700px
    :align: center
    Selecting the board and firmware release
    ```

    <br>

3. Set `OSD Protocol` to `Analog`.

    ```{figure} @assets@/en/assembly/clover5-fpv/firmware/build-osd-protocol.webp
    :alt: Selecting the Analog OSD protocol
    :width: 700px
    :align: center
    Selecting the Analog OSD protocol
    ```

    <br>

4. Click `Load Firmware [Online]` and wait for the build to complete.

    ```{figure} @assets@/en/assembly/clover5-fpv/firmware/build-load-firmware.webp
    :alt: Starting the firmware build
    :width: 700px
    :align: center
    Starting the firmware build
    ```

    <br>

5. Open the `Flash` tab. Once the build is successfully completed, click `Save` to download the HEX file to your computer. You will use this file in STM32CubeProgrammer.

    ```{figure} @assets@/en/assembly/clover5-fpv/firmware/build-save-hex.webp
    :alt: Saving the built HEX file
    :width: 700px
    :align: center
    Saving the built HEX file
    ```

    <br>

(clover5-fpv-dfu-mode)=

## How to Enter DFU Mode

To flash the firmware, you must first put the flight controller into DFU (Device Firmware Upgrade) mode:

1. Disconnect the flight controller from your computer.
2. Press and hold the `BOOT` button on the controller.
3. While continuing to hold the `BOOT` button, connect the controller to your computer via USB.
4. Confirm that your computer recognizes the controller in DFU mode. In **STM32CubeProgrammer**, the device should appear as either a DFU device or STM32 Bootloader.

## How to Install Betaflight Firmware

1. Open **STM32CubeProgrammer** and select the connection interface.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/select-connection-interface.webp
    :alt: Выбор интерфейса подключения в STM32CubeProgrammer
    :width: 700px
    :align: center
    Selecting connection interface
    ```

    <br>

2. Select the `USB` interface.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/select-usb-interface.webp
    :alt: Выбор интерфейса USB
    :width: 700px
    :align: center
    USB interface selected
    ```

    <br>

3. Click the Refresh button located to the right of the `Port` list. **STM32CubeProgrammer** should detect the controller in DFU mode. If the device does not appear, revisit the  {ref}`How to Enter DFU Mode<clover5-fpv-dfu-mode>` section.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/refresh-dfu-device.webp
    :alt: Поиск DFU-устройства
    :width: 700px
    :align: center
    Refreshing DFU device
    ```

    <br>

4. Click `Connect` to establish a link with the controller.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/connect-dfu-device.webp
    :alt: Подключение к контроллеру в режиме DFU
    :width: 700px
    :align: center
    Connecting DFU device
    ```

    <br>

5. Perform a full memory erase: Click the Eraser icon in the lower-left corner of the window, confirm the action in the pop-up, and wait for the process to finish.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/full-chip-erase.webp
    :alt: Полная очистка памяти контроллера
    :width: 700px
    :align: center
    Full erase
    ```

    <br>

6. Click `Open file` and select the downloaded `.hex` firmware file for **MicoAir743v2**. The filename depends on the firmware version.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/open-firmware-file.webp
    :alt: Выбор файла прошивки Betaflight
    :width: 700px
    :align: center
    Opening firmware file
    ```

    <br>

7. Click `Download` and wait for the writing process to complete.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/download-firmware.webp
    :alt: Загрузка прошивки Betaflight
    :width: 700px
    :align: center
    Writing Betaflight
    ```

    <br>

8. Once the writing is successful, click `Disconnect`.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/disconnect-programmer.webp
    :alt: Отключение контроллера от STM32CubeProgrammer
    :width: 700px
    :align: center
    Controller disconnected
    ```

    <br>

9. Disconnect the controller from the USB cable. Reconnect it without pressing the `BOOT` button. Open **Betaflight App**.

    ```{figure} @assets@/en/assembly/clover5-fpv/firmware/betaflight-configurator.webp
    :alt: Полётный контроллер в Betaflight Configurator
    :width: 700px
    :align: center
    Betaflight Configurator
    ```

    <br>

Once the firmware installation is complete, proceed to the {doc}`Betaflight Setup Guide <BetaflightSetup>`.
