# Setup Guide

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

Setup/Calibration
Setup/Radio
Setup/Modes
Setup/Power
```

```{figure} ../assets/common/setup/qgc.webp
:alt: QGroundControl
:width: 90%
:align: center

QGroundControl
```
<br>

## How to Install QGroundControl

QGroundControl (QGC) is the essential software used to flash, configure, and calibrate your flight controller.

Visit the [QGroundControl v5.1.4 release page](https://github.com/mavlink/QGroundControl/releases/tag/v5.1.4) and download the installer compatible with your operating system (Windows, Linux, or macOS).

Run the installer. If the installer prompts you to install additional drivers, ensure you accept them to allow proper communication with the flight controller.

For more in-depth information, refer to the official  [QGroundControl User Guide](https://docs.qgroundcontrol.com/).

## How to Upload Firmware

Before configuring your quadcopter, flash the latest stable firmware to the flight controller.

Download the stable PX4 1.16.1 firmware for MicoAir H743 V2: [micoair_h743-v2_default.px4](@assets@/downloads/micoair_h743-v2_default.px4)

Flashing Steps: 
1. Ensure the flight controller is disconnected from your computer.
2. Open the QGroundControl application.
3. Go to the `Vehicle Configuration` panel and select the `Firmware` menu.
4. Plug the flight controller into your computer using a USB cable.
5. In the QGC menu, select the corresponding serial port for your flight controller.
6. QGC will prompt you to enter bootloader mode. To do this, disconnect the USB cable and reconnect it.

```{figure} ../assets/common/setup/qgc-port-apply.webp
:alt: Переподключение полётного контроллера для загрузки прошивки
:width: 90%
:align: center

Flight Controller Reboot
```
<br>

8. Once QGC detects the controller, a window will appear. Select `PX4 Flight Stack`.
9. Open `Advanced settings`.
10. From the drop-down menu, select `Custom firmware file...`. Click `OK`.

```{figure} ../assets/common/setup/qgc-firmware.webp
:alt: Загрузка прошивки в QGroundControl
:width: 90%
:align: center

Firmware Uploaded to QGroundControl
```
<br>

11. Select the `micoair_h743-v2_default.px4` file you previously downloaded.

```{warning}
Do not disconnect the flight controller during the upload process. Interrupting this step may damage the firmware.
```

Wait for the upload to finish and for the flight controller to reboot automatically.

## Post-Installation Configuration

```{figure} ../assets/common/setup/qgc-requires-setup.webp
:alt: Обзор настроек QGroundControl
:width: 90%
:align: center

QGroundControl Configuration
```
<br>

Once the firmware is installed, you must configure the following sections in order:

1. *Airframe*. Define your frame geometry.
2. *Radio*. Set up your control equipment.
3. *Sensors*. Calibrate all onboard sensors.
4. *Flight Modes*. Define your flight control profiles.

### Frame Configuration

```{figure} ../assets/common/setup/qgc-frame-apply.webp
:alt: Выбор рамы в QGroundControl
:width: 90%
:align: center

Frame Geometry Configuration in QGroundControl
```
<br>
Setting the correct airframe ensures the flight controller understands how to apply motor outputs.

1. Navigate to the `Vehicle Configuration` panel.
2. Select the `Airframe` menu.
3. Choose `Quadrotor X` as the frame type.
4. Choose `Generic Quadcopter` as the subtype.
5. Scroll to the top of the list and click `Apply` and `Restart`.
6. Confirm the settings by clicking `Apply`.
7. Wait for the flight controller to complete the configuration and reboot.

### Parameter Configuration

Parameters fine-tune how the flight controller behaves.

1. Go to the `Vehicle Configuration` panel and select the `Parameters` menu.
2. Use the `Search` field to locate specific parameters.
3. Enter the required values.
4. You must click `Save` after changing each parameter to ensure the change is applied.

```{figure} ../assets/common/setup/qgc-parameters.webp
:alt: Параметры QGroundControl
:width: 90%
:align: center

QGroundControl Parameters
```
<br>

If changes do not take effect, go to the `Tools` menu and select `Reboot vehicle`.

To save time and ensure accuracy, you can load a pre-configured parameter file:

1. Navigate to the `Parameters` menu.
2. Click `Tools` and select `Load from file...`.
3. Select the appropriate file [clover5.params](@assets@/downloads/clover5.params).
4. Wait for the process to finish, then verify that the values have been applied correctly.

## PID Controller Tuning

The PID (Proportional-Integral-Derivative) controller manages the stability of your drone. 
For initial setup, use the following averaged coefficients:

* `MC_PITCHRATE_P` = 0.176
* `MC_PITCHRATE_I` = 0.213
* `MC_PITCHRATE_D` = 0.0018
* `MC_ROLLRATE_P` = 0.176
* `MC_ROLLRATE_I` = 0.213
* `MC_ROLLRATE_D` = 0.0018
* `MC_YAWRATE_P` = 0.25
* `MC_YAWRATE_I` = 0.09
* `MPC_XY_P` = 1.8
* `MPC_Z_P` = 1.5
* `MPC_XY_VEL_P_ACC` = 3.45
* `MPC_XY_VEL_D_ACC` = 0.15
* `MPC_XY_VEL_I_ACC` = 1.0
* `MPC_Z_VEL_P_ACC` = 5.5
* `MPC_Z_VEL_I_ACC` = 2.3
* `MPC_THR_HOVER` = 0.4
* `MPC_ACC_DOWN_MAX` = 2.0

```{tip}
While these averaged values provide a stable starting point, ideal flight performance requires manual PID tuning tailored to your specific quadcopter's weight, center of gravity, and motor efficiency.
```
