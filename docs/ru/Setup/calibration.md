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
Во время калибровки гироскопа квадрокоптер не должен менять положение, шататься или вибрировать.
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
2. Выберите ориентацию полетного контроллера *ROTATION_NONE*, если контроллер установлен передом к носу квадрокоптера.
3. Последовательно устанавливайте квадрокоптер в каждую ориентацию, которую показывает QGroundControl.
4. Держите квадрокоптер неподвижно до появления зеленой рамки.

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
2. Выберите ориентацию полетного контроллера *ROTATION_NONE*, если контроллер установлен передом к носу квадрокоптера.
3. Установите квадрокоптер на ровную поверхность.
4. Нажмите *OK*.
5. Дождитесь окончания калибровки.

Дополнительная информация: [PX4 Level Horizon Calibration](https://docs.px4.io/main/en/config/level_horizon_calibration.html).
