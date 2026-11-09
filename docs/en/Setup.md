# Setup Guide

```{toctree}
:titlesonly:
:maxdepth: 2
:hidden:

Setup/QGC
Setup/RPi
```

Before your first flight with the quadcopter, you must configure the flight controller and the onboard Raspberry Pi.

## Configuring the Flight Controller

To set up the {doc}`flight controller <Setup/QGC>`>, use the QGroundControl application. During this process, you will:
* Install the firmware.
* Calibrate sensors.
* Configure control peripherals and flight modes.
* Define power parameters.

## Configuring the Flight Controller

* First, flash the system image onto a memory card for the {doc}`Raspberry Pi <Setup/RPi>`. 
* Once powered, connect to the Raspberry Pi via Web IDE or SSH.
* Finally, prepare your ArUco marker map and use `clover2-settings` to configure the camera, navigation system, and other essential components.
