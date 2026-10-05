# Power System Configuration Guide

Proper voltage monitoring is essential to prevent battery damage and ensure safe flight times.

1. Open the `Vehicle Configuration` tab in QGroundControl.
2. Select the `Power` menu.
3. Connect your battery to the vehicle.
4. Set the `Number of cells` parameter to `6S`.
5. Connect a voltage indicator to the battery's balance connector.
6. Click `Calculate` next to the `Voltage Divider` parameter.
7. Enter the resulting value into the input field that appears.
8. Click `Close` to save the calibration.

If you do not have a voltage indicator or cannot perform manual calibration, set the `Voltage divider` value to `21`.

```{figure} ../../assets/common/setup/qgc-voltage-divider.webp
:alt: Калибровка делителя напряжения в QGroundControl
:width: 90%
:align: center

Power System Configuration
```
<br>

For more technical details, see:  [QGroundControl Power Setup](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/setup_view/power.html).
