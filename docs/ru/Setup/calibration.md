# Калибровка датчиков

Чтобы откалибровать датчики, откройте в QGroundControl вкладку *Vehicle Setup* и выберите меню *Sensors*.

## Гироскоп

```{figure} ../../assets/common/setup/qgc-cal-gyro.webp
:alt: Калибровка гироскопа в QGroundControl
:width: 90%
:align: center

Калибровка гироскопа
```
<br>

1. Выберите меню *Gyroscope*.
2. Установите квадрокоптер на ровную поверхность.
3. Нажмите *OK*.
4. Дождитесь окончания калибровки.

```{warning}
Во время калибровки гироскопа квадрокоптер должен стоять полностью неподвижно — его нельзя двигать.
```

Дополнительная информация: [PX4 Gyroscope Calibration](https://docs.px4.io/main/en/config/gyroscope.html).

## Акселерометр

```{figure} ../../assets/common/setup/qgc-cal-acc.webp
:alt: Калибровка акселерометра в QGroundControl
:width: 90%
:align: center

Калибровка акселерометра
```
<br>

1. Выберите меню *Accelerometer*.
2. Выберите ориентацию полётного контроллера *ROTATION_NONE*, если стрелка на контроллере смотрит вперёд, в сторону носа квадрокоптера.
3. Расположите квадрокоптер в показанном на экране положении и переходите к следующему по подсказкам QGroundControl.
4. Держите квадрокоптер неподвижно, пока рамка на экране не станет зелёной.

Дополнительная информация: [PX4 Accelerometer Calibration](https://docs.px4.io/main/en/config/accelerometer.html).

## Уровень горизонта

```{figure} ../../assets/common/setup/qgc-cal-level.webp
:alt: Калибровка горизонта в QGroundControl
:width: 90%
:align: center

Калибровка горизонта
```
<br>

1. Выберите меню *Level Horizon*.
2. Выберите ориентацию полётного контроллера *ROTATION_NONE*, если стрелка на контроллере смотрит вперёд, в сторону носа квадрокоптера.
3. Установите квадрокоптер на ровную поверхность.
4. Нажмите *OK*.
5. Дождитесь окончания калибровки.

Дополнительная информация: [PX4 Level Horizon Calibration](https://docs.px4.io/main/en/config/level_horizon_calibration.html).
