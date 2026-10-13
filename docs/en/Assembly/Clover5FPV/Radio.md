# Radio Controller Setup Guide

This guide describes how to set up the **RadioMaster Pocket ELRS** controller and bind it with the **ExpressLRS** receiver.

```{caution}
Before configuring, remove the propellers from the quadcopter to prevent the motors from starting accidentally.
```

```{contents}
:local:
:depth: 2
```

## Controller Overview

```{figure} @assets@/en/setup/radio/rc-pocket-front-part.webp
:alt: RadioMaster Pocket, front view
:width: 90%
:align: center
Controller front view
```

```{figure} @assets@/en/setup/radio/rc-pocket-back-part.webp
:alt: RadioMaster Pocket, rear view
:width: 90%
:align: center
Controller rear view
```

## Preparing the Controller

1. Remove the black rubber pads from the controller back.
2. Insert the battery into the designated compartment. Check the polarity (+/-).
3. Reinstall the rubber pads.
4. Press and hold the `POWER` button to turn on the controller.
5. If a `WARNING` appears on the screen, set the **Throttle** stick to its lowest position and move all switches to their neutral/zero positions.


```{figure} @assets@/common/setup/radio/warn.webp
:alt: Предупреждение о положении стиков или тумблеров
:width: 55%
:align: center
Warning screen
```

6. Once the warning is cleared, navigate to the main menu.

```{figure} @assets@/common/setup/radio/main-menu-no-signal.webp
:alt: Главное меню аппаратуры без связи с приемником
:width: 55%
:align: center
Main menu
```

## Configuring a Model for Betaflight

```{note}
If the **Clover 5 FPV** model is already configured, verify the **Internal RF** mode and **Channel assignment** in the **MIXES** menu before proceeding to binding.
```

You may either create a model manually or load a pre-configured file.

### Creating a Model Manually

1. Press the `MDL` button.
2. Select an empty slot and press and hold the rotary encoder.

    ```{figure} @assets@/common/setup/radio/model/create-model-menu.webp
    :alt: Выбор свободной ячейки модели
    :width: 450px
    :align: center
    Creating model
    ```

3. Select `CREATE MODEL`.
4. Press `PAGE`,  use the rotary encoder to select `Model name` and name your model (e.g. `CLOVER FPV`).

    ```{figure} @assets@/common/setup/radio/model/model-name.webp
    :alt: Настройка имени модели квадрокоптера
    :width: 450px
    :align: center
    Model naming
    ```

5. Press `RTN` and scroll down to the `Internal RF` section. Set the `Mode` to `CRSF`.

    ```{figure} @assets@/common/setup/radio/model/internal-rf-disabled.webp
    :alt: Выключенный внутренний радиомодуль
    :width: 450px
    :align: center
    Internal RF disabled
    ```

    ```{figure} @assets@/common/setup/radio/model/internal-rf-crsf.webp
    :alt: Внутренний радиомодуль в режиме CRSF
    :width: 450px
    :align: center
    CRSF mode
    ```

6. Navigate to the `MIXES` page and assign the following switches to the additional channels:

   - `CH5 / AUX1` — `SA`;
   - `CH6 / AUX2` — `SB`;
   - `CH7 / AUX3` — `SC`;
   - `CH8 / AUX4` — `SD`.

    ```{figure} @assets@/common/setup/radio/model/mix-channel-settings.webp
    :alt: Настройка переключателя на дополнительном канале
    :width: 450px
    :align: center
    Channel settings
    ```

    ```{figure} @assets@/common/setup/radio/model/mixes-channels.webp
    :alt: Основные и дополнительные каналы модели
    :width: 450px
    :align: center
    Model channels
    ```

    ```{figure} @assets@/common/setup/radio/model/mixes-switches.webp
    :alt: Список настроенных переключателей
    :width: 450px
    :align: center
    Switches
    ```

7. Press `RTN`. After binding, open the **Betaflight Configurator** and check the `Receiver` tab to ensure that moving the sticks and switches updates the corresponding channels correctly.

### Loading a Pre-configured Model

To save time, you can load a pre-configured profile:

1. Download the model file: [`model05.yml`](https://drive.google.com/file/d/19Ab9sWPvaOvIavkEXPXzDXCRajpBcO7e/view?usp=sharing).
2. Power on your controller and connect it to your computer via a USB-C cable using the top USB port. Select `USB Storage (SD)` on the device.
3. Open the device's storage on your computer and move to the `MODELS` directory.

    ```{figure} @assets@/common/setup/radio/model/usb-storage-models-folder.webp
    :alt: Каталог MODELS на карте памяти аппаратуры
    :width: 600px
    :align: center
    Models folder
    ```

5. Copy the `model05.yml` file into the `MODELS` folder.

    ```{figure} @assets@/common/setup/radio/model/model-file-copied.webp
    :alt: Файл готовой модели в каталоге MODELS
    :width: 600px
    :align: center
    Model file
    ```

6. Safely eject the storage, disconnect the cable, and select the newly loaded model from the `MDL` menu.
7. Confirm that `Internal RF` is set to `CRSF` mode and that switches are correctly assigned to `CH5–CH8`.

## Binding Using a Binding Phrase

The **Binding Phrase** must be identical on both the transmitter and the receiver. This method requires ExpressLRS 3.0 or newer. If your devices are running older firmware, update them via the [ExpressLRS Web Flasher](https://expresslrs.github.io/web-flasher/) before proceeding.

### Transmitter Setup

1. Power on the control equipment and press `SYS`.
2. Open the `ExpressLRS` tool.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Инструмент ExpressLRS в системном меню
    :width: 450px
    :align: center
    ExpressLRS
    ```

3. Select `WiFi Connectivity`, then select `Enable WiFi`. Wait until the `WiFi Running` message appears.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-connectivity-menu.webp
    :alt: Пункт WiFi Connectivity в ExpressLRS
    :width: 450px
    :align: center
    WiFi Connectivity in ExpressLRS
    ```

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-enable-wifi.webp
    :alt: Включение Wi-Fi передатчика ExpressLRS
    :width: 450px
    :align: center
    Wi-Fi enabling
    ```

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-running.webp
    :alt: Передатчик ExpressLRS в режиме Wi-Fi
    :width: 450px
    :align: center
    Wi-Fi running
    ```

4. On your computer or smartphone, connect to the `ExpressLRS TX` Wi-Fi network (Default password: `expresslrs`).

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS TX
    :width: 450px
    :align: center
   Connecting to ExpressLRS TX
    ```

5. Open a web browser and go to:  `http://10.0.0.1`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-web-interface.webp
    :alt: Веб-интерфейс передатчика ExpressLRS
    :width: 450px
    :align: center
   Web interface
    ```

6. In the `Binding Phrase` field, enter your chosen phrase and click `SAVE`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-binding-phrase.webp
    :alt: Binding Phrase в настройках передатчика
    :width: 700px
    :align: center
    ```

7. Click `REBOOT` to confirm.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Перезагрузка устройства ExpressLRS
    :width: 450px
    :align: center
    ExpressLRS reboot
    ```

### Receiver setup

```{caution}
Before powering the receiver for an extended period, ensure the antenna is connected to the Video Transmitter (VTX) to prevent damage. You may temporarily disconnect the VTX from the flight controller to keep it cool. Never install propellers during this process.
```

1. Turn off the controller to prevent connection attempt.
2. Power on the flight controller and wait approximately 60 seconds. When the receiver enters Wi-Fi mode, its LED will begin blinking rapidly.
3. Connect your computer or smartphone to the `ExpressLRS RX` Wi-Fi network (Default password: `expresslrs`).

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS RX
    :width: 450px
    :align: center
    Connecting to ExpressLRS RX
    ```

4. Open a web browser and go to: `http://10.0.0.1`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-web-interface.webp
    :alt: Веб-интерфейс приемника ExpressLRS
    :width: 450px
    :align: center
    Web interface
    ```

5. Enter the **exact same phrase** used on the transmitter in the `Binding Phrase` field. Click `SAVE & REBOOT`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-binding-phrase.webp
    :alt: Binding Phrase в настройках приемника
    :width: 700px
    :align: center
    Binding Phrase
    ```

6. When the confirmation window appears, click `REBOOT` and wait for the receiver to restart.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Подтверждение перезагрузки приемника
    :width: 450px
    :align: center
    Reboot
    ```

7. Turn on your controller. Once connected, a receiver indicator will appear on the main screen.

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после сопряжения с приёмником
    :width: 55%
    :align: center
    ```

## Binding in Bind Mode

If you cannot use a Binding Phrase, use the manual **Bind Mode**:

1. Turn the ELRS receiver into bind mode by connecting and disconnecting power to the flight controller three times in quick succession. The receiver LED should begin blinking in a specific pattern (two blinks followed by a pause).

3. On the controller, press `SYS` and open the `ExpressLRS tool` to initiate the binding process.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Переход в меню ExpressLRS
    :width: 55%
    :align: center
    ExpressLRS menu
    ```

4. Set `TX Power` to `100 mW`.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-2.webp
    :alt: Мощность передатчика 100 mW
    :width: 55%
    :align: center
    TX Power
    ```

5. Select `Bind` and wait for binding to complete.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-3.webp
    :alt: Запуск сопряжения ExpressLRS
    :width: 55%
    :align: center
    ExpressLRS binding start
    ```

    ```{figure} @assets@/common/setup/radio/rx/elrs-bind.webp
    :alt: Процесс сопряжения ExpressLRS
    :width: 55%
    :align: center
    ExpressLRS binding
    ```

6. Press and hold the `RTN` button to return to the main menu. Make sure that a connection indicator with the receiver is displayed on the main screen.

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после успешного сопряжения
    :width: 55%
    :align: center
    Receiver bound
    ```

After binding, open the `Receiver` tab in **Betaflight Configurator** and check the operation of the sticks and all assigned switches.

For more details on binding methods, see the [ExpressLRS](https://www.expresslrs.org/quick-start/binding/).
