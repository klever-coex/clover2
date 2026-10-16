# Control Equipment Setup

This section details how to prepare your transmitter and pair it with an ExpressLRS (ELRS) receiver.

```{caution}
Remove all propellers before starting this setup. This prevents accidental motor starts during channel and switch testing.
```

## Hardware Preparation

```{figure} @assets@/ru/setup/radio/rc-pocket-front-part.webp
:alt: Аппаратура управления, вид спереди
:width: 90%
:align: center

Transmitter front view
```
```{figure} @assets@/ru/setup/radio/rc-pocket-back-part.webp
:alt: Аппаратура управления, вид сзади
:width: 90%
:align: center

Transmitter rear view
```

1. Remove the black rubber pads from the rear of the transmitter.
2. Install the battery, ensuring correct polarity.
3. Reinstall the rubber pads.
4. Press and hold the **POWER** button to turn on the device.

```{caution}
If the screen displays a WARNING message, immediately move the Throttle stick to the lowest position.
```

```{figure} @assets@/common/setup/radio/warn.webp
:alt: Предупреждение при неверном положении стиков или тумблеров
:width: 55%
:align: center

Warning
```
5. Once the warning is cleared, proceed to the main menu.

```{figure} @assets@/common/setup/radio/main-menu-no-signal.webp
:alt: Главное меню аппаратуры без связи с приёмником
:width: 55%
:align: center

Main menu
```

## Pairing via Binding Phrase (Recommended)

### Transmitter (TX) Setup

1. Press the `SYS` button
2. Navigate to the `ExpressLRS` menu.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Инструмент ExpressLRS в системном меню
    :width: 55%
    :align: center
    
    ExpressLRS
    ```

3. Select `WiFi Connectivity`.
  
    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-connectivity-menu.webp
    :alt: Пункт WiFi Connectivity в ExpressLRS
    :width: 450px
    :align: center
   WiFi Connectivity in ExpressLRS 
    ```

4. Select `Enable WiFi`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-enable-wifi.webp
    :alt: Включение Wi-Fi передатчика ExpressLRS
    :width: 450px
    :align: center
    Wi-Fi enabled
    ```
5. Wait for the `WiFi Running` message.
    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-running.webp
    :alt: Передатчик ExpressLRS в режиме Wi-Fi
    :width: 450px
    :align: center
    WiFi Running
    ```

6. On your computer or smartphone, connect to the `ExpressLRS TX` network (Default password: `expresslrs`).

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS TX
    :width: 450px
    :align: center
    ExpressLRS TX
    ```

7. Open a web browser and go to http://10.0.0.1 (http://10.0.0.1/).

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-web-interface.webp
    :alt: Веб-интерфейс передатчика ExpressLRS
    :width: 450px
    :align: center
    ExpressLRS web interface
    ```

8. Enter your chosen phrase in the `Binding Phrase` field and click `SAVE`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/tx-binding-phrase.webp
    :alt: Binding Phrase в настройках передатчика
    :width: 700px
    :align: center
    Binding Phrase 
    ```

9. Click `REBOOT` to apply changes.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Перезагрузка устройства ExpressLRS
    :width: 450px
    :align: center
    ExpressLRS Reboot
    ```

### Receiver (RX) Setup

```{caution}
If your Video Transmitter (VTX) powers on with the flight controller, ensure an antenna is connected and provide active cooling, or temporarily disconnect the VTX. Do not install propellers.
```

1. Turn OFF the transmitter to prevent accidental binding.
2. Power on the flight controller and wait ~60 seconds. The receiver's LED will blink rapidly when it enters Wi-Fi mode.
3. Connect your computer/smartphone to the `ExpressLRS RX network` (Default password: `expresslrs`).

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-wifi-network.webp
    :alt: Подключение к сети ExpressLRS RX
    :width: 450px
    :align: center
    ExpressLRS RX
    ```

4. Open http://10.0.0.1 (http://10.0.0.1/) in your browser.

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-web-interface.webp
    :alt: Веб-интерфейс приёмника ExpressLRS
    :width: 450px
    :align: center
    ExpressLRS web interface
    ```

5. Enter the exact same phrase used on the transmitter. 

    ```{figure} @assets@/common/setup/radio/bind-phrase/rx-binding-phrase.webp
    :alt: Binding Phrase в настройках приёмника
    :width: 700px
    :align: center
    Binding Phrase 
    ```

6. Click `SAVE & REBOOT`, then confirm by clicking `REBOOT`.

    ```{figure} @assets@/common/setup/radio/bind-phrase/web-interface-reboot.webp
    :alt: Подтверждение перезагрузки приёмника
    :width: 450px
    :align: center
    Receiver reboot
    ```

7. Turn on your transmitter. A connection indicator should now appear on the transmitter's main screen.

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после сопряжения с приёмником
    :width: 55%
    :align: center
   Main menu
    ```

### Alternative Pairing: Manual Bind Mode

Use this method if you are unable to configure a Binding Phrase.

1. Enter Receiver Bind Mode: Power the flight controller on and off three times in quick succession. The receiver LED should blink in a specific pattern (two blinks followed by a pause).
2. On the transmitter, press `SYS` and open the `ExpressLRS` menu.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-1.webp
    :alt: Переход в меню ExpressLRS
    :width: 55%
    :align: center
   ExpressLRS menu
    ```

3. Set `TX Power` to `100 mW`.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-2.webp
    :alt: Мощность передатчика 100 mW
    :width: 55%
    :align: center
   TX Power setting
    ```

4. Select the `Bind` command and wait for the process to complete.

    ```{figure} @assets@/common/setup/radio/rx/elrs-step-3.webp
    :alt: Запуск сопряжения ExpressLRS
    :width: 55%
    :align: center
    ExpressLRS menu
    ```

5. Once the connection status is displayed, press and hold `RTN` to return to the main menu.
  
    ```{figure} @assets@/common/setup/radio/rx/elrs-bind.webp
    :alt: Процесс сопряжения ExpressLRS
    :width: 55%
    :align: center
    Binding
    ```

6. Verify that the connection indicator is visible on the main screen.

    ```{figure} @assets@/common/setup/radio/main-menu.webp
    :alt: Главное меню после успешного сопряжения
    :width: 55%
    :align: center
   Binded
    ```
