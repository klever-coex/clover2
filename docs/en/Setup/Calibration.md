# Sensor Calibration Guide

To begin, navigate to the `Vehicle Configuration` panel and select the `Sensors` menu.

```{caution}
Before starting any calibration, ensure the vehicle is placed on a completely stable, level surface.
```

## How to Calibrate Gyroscope

```{figure} ../../assets/common/setup/qgc-cal-gyro.webp
:alt: Калибровка гироскопа в QGroundControl
:width: 90%
:align: center

Gyroscope Calibration
```
<br>

1. Select the `Gyroscope` menu.
2. Click `OK`.
3. Do not move the vehicle until the `Calibration Successful` message appears.

For more technical details, see:  [PX4 Gyroscope Calibration](https://docs.px4.io/main/en/config/gyroscope.html).

## How to Calibrate Accelerometer 

```{figure} ../../assets/common/setup/qgc-cal-acc.webp
:alt: Калибровка акселерометра в QGroundControl
:width: 90%
:align: center

Accelerometer Calibration
```
<br>

1. Select the `Accelerometer` menu.
2. If the flight controller is mounted with its arrow pointing toward the nose of the vehicle, select `ROTATION_NONE`.
3. QGroundControl will guide you through several required positions. Place the vehicle in the position indicated by the software.
4. Once the yellow frame appears, hold the vehicle steady in that exact position.
5. Do not move the vehicle until the frame turns green.
6. Follow the prompts until all required positions are completed.

For more technical details, see: [PX4 Accelerometer Calibration](https://docs.px4.io/main/en/config/accelerometer.html).

## How to Calibrate Level Horizon

```{figure} ../../assets/common/setup/qgc-cal-level.webp
:alt: Калибровка горизонта в QGroundControl
:width: 90%
:align: center

Level Horizon Calibration
```
<br>

1. Select the `Level Horizon` menu.
2. Select `ROTATION_NONE` if the controller arrow points toward the nose.
3. Click `OK`.
4. Do not move the vehicle until the process completes and the success message is displayed.

For more technical details, see: [PX4 Level Horizon Calibration](https://docs.px4.io/main/en/config/level_horizon_calibration.html).
