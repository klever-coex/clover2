# Setup Guide

This guide covers the basic setup of the **Klever 5 FPV** quadcopter with a **MicoAir743v2** flight controller in the **Betaflight Configurator**.

```{tip}
Make sure the Betaflight firmware is installed on the controller before you start the setup.
```

```{caution}
Remove the propellers before configuring and testing the motors.
```

## Connection and Preparation

1. Connect your flight controller to your computer using a USB cable and launch the **Betaflight Configurator**.
2. Click `Connect` to establish a link with the flight controller.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/configurator-connection.webp
    :alt: Подключение полётного контроллера в Betaflight Configurator
    :width: 700px
    :align: center
    Flight controller to Betaflight Configurator connection
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/setup-overview.webp
    :alt: Setup tab overview
    :width: 700px
    :align: center
    Setup tab overview
    ```

    <br>

## Calibration and Basic Configuration

1. If you see an `Accelerometer is not calibrated` warning, navigate to the `Sensors` tab and click `Calibrate`.
2. Disable any sensors that you do not use.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/accelerometer-calibration.webp
    :alt: Калибровка акселерометра
    :width: 700px
    :align: center
    Accelerometer calibration
    ```

    <br>

     ```{figure} @assets@/en/assembly/clover5-fpv/setup/basic-configuration.webp
    :alt: Базовая конфигурация квадрокоптера
    :width: 700px
    :align: center
    Basic configuration
    ```

    <br>

3. Navigate to the `Configuration` tab. In the `Personalization` section:
* Enter your quadcopter's name in the `Aircraft name` field.
* Enter the pilot's name (optional) in the `Pilot name` field.

4. In the `Other Features` section, enable the following: `AIRMODE`, `DISPLAY`, `LED_STRIP`, `OSD`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/feature-configuration.webp
    :alt: Включение дополнительных функций Betaflight
    :width: 700px
    :align: center
    Feature configuration
    ```

    <br>

5. Click `Save and Reboot`.

## Port Configuration

1. Open the `Ports` tab.
2. Configure your UARTs as follows:

   - **UART2**: Disable `MSP`. In the `Peripherals` section, select `VTX (IRC Tramp)`.
   - **UART3**: If using GPS, set `Sensor Input` to `GPS` an the speed to `57600`.
   - **UART6**: Enable `Serial RX` for your receiver.
   - **UART7**: If using ESC telemetry, select `ESC`.

3. Click `Save and Reboot`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/port-configuration.webp
    :alt: Настройка портов полётного контроллера
    :width: 700px
    :align: center
    UART configuration
    ```

    <br>

```{caution}
If your receiver, GPS, or VTX are connected to different UART ports, ensure you apply these settings to the corresponding UARTs used in your actual hardware setup.
```

## Receiver Configuration

1. Navigate to the `Receiver` tab.
2. Set `Receiver Mode` to `Serial (via UART)`.
3. Set `Serial Receiver Provider` to `CRSF`.
4. Use your transmitter sticks to verify correct movement in the preview area.
5. In the `Channel Map` tab, select `AETR1234`.
6. If the receiver is not yet linked, use the `Bind Receiver` button.
7. Click `Save`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/receiver-configuration.webp
    :alt: Настройка приёмника CRSF
    :width: 700px
    :align: center
    CRSF receiver configuration
    ```

    <br>

## Mode Configuration

1. Navigate to the `Modes` tab.
2. Configure the following switches:

   - `ARM`: Set to `AUX1` (Range: `1675–2075`)
   - `ANGLE`: Set to `AUX3` (Range: `1300–1700`)
   - `HORIZON`: Set to `AUX3` (Range: `1700–2100`)
   - `FAILSAFE`: Set to `AUX4` (Range: `1700–2100`)
   - `BEEPER`: Set to `AUX4` (Range: `1700–2100`)

3. Click `Save`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/flight-modes.webp
    :alt: ARM, ANGLE, and HORIZON modes
    :width: 700px
    :align: center
    ARM, ANGLE, and HORIZON modes
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/failsafe-beeper-modes.webp
    :alt: Настройка режимов FAILSAFE и BEEPER
    :width: 700px
    :align: center
    ```

    <br>

    If needed, assign `FLIP OVER AFTER CRASH` to `AUX2` with a range of `1700–2100`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/flip-over-after-crash-mode.webp
    :alt: Flip over after crash mode
    :width: 700px
    :align: center
    Flip over after crash mode
    ```

    <br>

## Motor Configuration

```{caution}
Ensure all propellers are removed before proceeding with motor configuration and testing.
```

1. Navigate to the `Motors` tab.
2. Set the `Mixer` to `QUAD X`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/quad-x-mixer.webp
    :alt: Выбор схемы QUAD X
    :width: 700px
    :align: center
    QUAD X selected
    ```

    <br>

3. In the `ESC/Motor Features` section, apply these settings:

   - `ESC/Motor protocol`: `DSHOT600`
   - `ESC_SENSOR`: *Enabled*
   - `Bidirectional DShot`: *Enabled*
   - `Motor poles`: `14`
   - `Motor Idle (% static)`: `5.5`

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/motor-features.webp
    :alt: Настройка ESC и моторов
    :width: 700px
    :align: center
    ESC and motors configuration
    ```

    <br>

4. Enable `Motor direction is reversed`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/reversed-motor-direction.webp
    :alt: Выбор обратного направления вращения моторов
    :width: 700px
    :align: center
    ```

    <br>

5. Click `Motor direction` to configure how each motor spins.
6. Acknowledge the warning regarding the removal of propellers and select `Wizard`.
   
    ```{figure} @assets@/en/assembly/clover5-fpv/setup/motor-direction-method.webp
    :alt: Safety notice and motor direction method
    :width: 700px
    :align: center
    Safety notice and motor direction method
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/motor-direction-wizard.webp
    :alt: Starting the motor direction check
    :width: 700px
    :align: center
    Starting the motor direction check
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/motor-direction-adjustment.webp
    :alt: Adjusting motor directions
    :width: 700px
    :align: center
    Adjusting motor directions
    ```

    <br>

8. Click `Reorder motors` to ensure the motor numbering matches your frame layout.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/motor-reordering-start.webp
    :alt: Запуск настройки порядка моторов
    :width: 700px
    :align: center
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/motor-reordering.webp
    :alt: Настройка порядка моторов
    :width: 700px
    :align: center
    ```

    <br>

9. If motors spin in the incorrect direction, use the `Wizard` to adjust them.

## OSD Configuration

1. Navigate to the `OSD` tab.
2. For analog video systems, set the format to `Auto`.
3. In the `Units` section, select `Metric`.
4. Configure your timers as follows:

   - **Timer 1**: `On time`
   - **Timer 2**: `Total armed time`

5. In the `Alarms` section, set the following values:

   - `RSSI`: `20`
   - `Capacity`: `2200`
   - `Altitude`: `100`
   - `Link Quality`: `60`

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/osd-settings.webp
    :alt: Настройка параметров OSD
    :width: 700px
    :align: center
    OSD configuration
    ```

    <br>

6. Enable and drag the following elements onto your screen layout to your preference:

   - `Aircraft name`
   - `Battery voltage`
   - `Disarmed`
   - `Fly mode`
   - `GPS latitude`
   - `GPS longitude`
   - `Throttle position`
   - `Timer 1`
   - `Timer 2`
   - `VTX channel`
   - `Warnings`

7. Once you have organized the layout, click `Save`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/osd-layout.webp
    :alt: Размещение элементов OSD
    :width: 700px
    :align: center
    OSD layout
    ```

    <br>

## Video Transmission System Configuration

```{caution}
Always connect the antenna to the video transmitter (VTX) before applying power. Operating a VTX without an antenna can cause permanent hardware damage. Additionally, ensure you only use frequencies and power levels permitted by your local regulations.
```

1. Open the `Video Transmitter` tab.
2. If the VTX table is not present, click `Load from file`.
3. Select your specific VTX table file.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/vtx-settings.webp
    :alt: Настройка видеопередатчика
    :width: 700px
    :align: center
    VTX configuration
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/vtx-table.webp
    :alt: Таблица частот и мощности видеопередатчика
    :width: 700px
    :align: center
    ```

    <br>

4. Once the table is loaded, verify and set the following:

   - `Band`: `RACEBAND`
   - `Channel`: `Channel 5`
   - `Power`: `25`
   - `Pit Mode`: *Enable as required for your workflow.*
   - `Low Power Disarm`: `Off`

5. Confirm that the frequency and power levels have been set correctly.
6. Select `Save`.

## LED Strip Configuration

1. Ensure the `LED_STRIP` feature was previously enabled in the `Configuration` tab.
2. Navigate to the `LED Strip` tab.
3. Use the grid to mark the specific LEDs.
4. Click `Wire Ordering Mode` to define the LED sequence.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/led-strip-layout.webp
    :alt: Разметка светодиодов на сетке
    :width: 700px
    :align: center
    LEDs layout
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/led-strip-wire-order-mode.webp
    :alt: Wire Ordering Mode
    :width: 700px
    :align: center
    Wire Ordering Mode
    ```

    <br>

5. In the `Function` field, select `Color`.
6. If desired, select a modifier (e.g., `Throttle`) in the `Color Modifier` section to link LED colors to flight inputs.
7. Assign your preferred colors and click `Save`.

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/led-strip-wire-order.webp
    :alt: Настройка порядка светодиодов
    :width: 700px
    :align: center
    LED order
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/led-strip-color-settings.webp
    :alt: Настройка цвета LED-ленты
    :width: 700px
    :align: center
    LED colors
    ```

    <br>

    ```{figure} @assets@/en/assembly/clover5-fpv/setup/led-strip-preview.webp
    :alt: Предварительный просмотр LED-ленты
    :width: 700px
    :align: center
    LED preview
    ```

    <br>

## Final Setup and Pre-Flight Checklist

1. In the `Setup` tab, confirm the quadcopter model icon moves in the same direction as your physical frame.
2. In the `Receiver` tab, verify that all stick movements and switch toggles are displayed accurately.
3. In the `Modes` tab, cycle through your switches to ensure the correct ranges are activated for each mode. 
5. Verify the correct motor rotation direction (ensure **propellers are removed**).
6. Confirm that the OSD elements are clearly visible and correctly positioned in your FPV goggles.

Install propellers only after all the above checks are successfully completed and before your first test flight.
