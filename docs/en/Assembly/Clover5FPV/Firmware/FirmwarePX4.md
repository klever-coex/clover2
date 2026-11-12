# Switching to PX4

This guide describes how to replace the Betaflight firmware by PX4 on a **MicoAir743v2** flight controller using DFU mode.

```{caution}
This guide applies only to the **MicoAir743v2** flight controller.
A full memory clearance will delete the installed Betaflight firmware and its settings.
Before you proceed, make sure you have selected the correct controller and firmware file.
```

```{figure} @assets@/ru/assembly/clover5-fpv/firmware/micoair743v2.webp
:alt: Полетный контроллер MicoAir743v2
:width: 350px
:align: center

MicoAir743v2 flight controller
```

<br>

## Required Software and Files

Prepare the following before proceeding:

- [QGroundControl](https://github.com/mavlink/QGroundControl/releases) — app used to build PX4 firmware and configure your flight controller.
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) — utility required to clear the controller memory and flash new firmware.
- [PX4 firmware for MicoAir743v2](@assets@/downloads/micoair_h743-v2_default.px4) (`.px4` format) — to be installed via QGroundControl.
- [PX4 bootloader for MicoAir743v2](@assets@/downloads/micoair_h743-v2_bootloader.bin) (`.bin` format) — to be written via STM32CubeProgrammer prior to firmware installation.

Ensure **STM32CubeProgrammer** and **QGroundControl** are fully installed before flashing the device.

## How to Write the PX4 Bootloader

1. Press and hold the `BOOT` button on the controller. While continuing to hold the `BOOT` button, connect the controller to your computer via USB.

2. Open **STM32CubeProgrammer** and select the connection interface.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/select-connection-interface.webp
    :alt: Выбор интерфейса подключения в STM32CubeProgrammer
    :width: 700px
    :align: center

    Selecting connection interface
    ```

    <br>

3. Select the `USB` interface.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/select-usb-interface.webp
    :alt: Выбор интерфейса USB
    :width: 700px
    :align: center

    USB interface selected
    ```

    <br>

4. Click the Refresh button located to the right of the `Port` list. **STM32CubeProgrammer** should detect the controller in DFU mode. If the port is not detected, reconnect the controller while continuing to hold the `BOOT` button.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/refresh-dfu-port.webp
    :alt: Поиск DFU-устройства
    :width: 700px
    :align: center

   Refreshing DFU device
    ```

    <br>

5. Click `Connect` to establish a link with the controller.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/connect-dfu-device.webp
    :alt: Подключение к контроллеру в режиме DFU
    :width: 700px
    :align: center

    Connecting DFU device
    ```

    <br>

6. Perform a full memory erase: click the `Eraser` icon in the lower-left corner of the window, confirm the action in the pop-up, and wait for the process to finish.


    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/erase-flash.webp
    :alt: Полная очистка памяти контроллера
    :width: 700px
    :align: center

    Full erase
    ```

    <br>

6. Click `Open file` and select the downloaded `micoair_h743-v2_bootloader.bin` file.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/open-bootloader-file.webp
    :alt: Выбор файла загрузчика PX4
    :width: 700px
    :align: center

    Opening bootloader file
    ```

    <br>

7. Click `Download` and wait for the writing process to complete.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/write-bootloader.webp
    :alt: Запись загрузчика PX4
    :width: 700px
    :align: center

    Writing PX4 bootloader
    ```

    <br>

8. Once the writing is successful, click `Disconnect`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/disconnect-programmer.webp
    :alt: Отключение контроллера от STM32CubeProgrammer
    :width: 700px
    :align: center

   Controller disconnected
    ```

    <br>

9.  Close **STM32CubeProgrammer** and disconnect the controller from your computer.

## How to Install the PX4 Firmware

1. Open **QGroundControl**.
2. Connect the flight controller to your computer via USB (do not press the `BOOT` button this time).
3. Click the **QGroundControl** logo located in the upper-left corner.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/open-qgc-menu.webp
    :alt: Главное меню QGroundControl
    :width: 700px
    :align: center

    QGroundControl main menu
    ```

    <br>

4. Select `Configure` from the drop-down menu to open the `Vehicle Configuration` panel.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/open-vehicle-configuration.webp
    :alt: Пункт Configure в выпадающем меню QGroundControl
    :width: 700px
    :align: center

    Configure in QGroundControl
    ```

    <br>

5. Select the `Firmware` section.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/open-firmware-menu.webp
    :alt: Переход в меню Firmware
    :width: 700px
    :align: center

    Opening Firmware menu
    ```

    <br>

6. Select your controller from the port list and click `Flash`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/start-firmware-update.webp
    :alt: Выбор контроллера и запуск прошивки
    :width: 700px
    :align: center

    Selecting controller
    ```

    <br>

7. If QGroundControl prompts you to reconnect the controller, follow the on-screen instructions (remember: do not press the `BOOT` button).

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/reconnect-controller.webp
    :alt: Запрос на переподключение контроллера
    :width: 700px
    :align: center

    Reconnecting controller
    ```

    <br>

8. In the `Firmware Setup` window, enable `Advanced settings`, select `Custom firmware file...` and click `OK`. Locate and seelect the `micoair_h743-v2_default.px4` file.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/select-custom-firmware.webp
    :alt: Выбор прошивки из файла
    :width: 700px
    :align: center

   Selecting firmware from file
    ```

    <br>

9. Wait for the firmware writing process to complete.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/wait-for-firmware-upload.webp
    :alt: Запись прошивки PX4
    :width: 700px
    :align: center

   Uploading PX4
    ```

    <br>

10. Next Steps: proceed to  {doc}`configuring the flight controller <../../../Setup/QGC>`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/configure-flight-controller.webp
    :alt: Полетный контроллер после установки PX4
    :width: 700px
    :align: center

    Flight controller with PX4 installed
    ```
