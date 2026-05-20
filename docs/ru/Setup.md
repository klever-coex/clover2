# Настройка

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

Setup/calibration
Setup/radio
Setup/modes
Setup/power
```

```{figure} ../assets/common/setup/qgc.webp
:alt: QGroundControl
:width: 90%
:align: center

QGroundControl
```
<br>

## Установка QGroundControl

**QGroundControl** - программа для прошивки, настройки и калибровки полетного контроллера Клевера.

Скачайте и установите QGroundControl для Windows, Linux или macOS с [официального сайта](https://qgroundcontrol.com/downloads/). Если установщик предложит поставить дополнительные драйверы, согласитесь с установкой.

Дополнительная документация доступна на сайте [QGroundControl User Guide](https://docs.qgroundcontrol.com/).

## Загрузка прошивки в полетный контроллер

Скачайте актуальную стабильную версию PX4 на странице [PX4 Autopilot releases](https://github.com/PX4/PX4-Autopilot/releases).

Для загрузки прошивки:

1. Отключите полетный контроллер от компьютера, если он уже подключен.
2. Запустите QGroundControl.
3. Перейдите в панель *Vehicle Configuration* и выберите меню *Firmware*.
4. Подключите полетный контроллер к компьютеру по USB.
5. Выберите *PX4 Flight Stack*.

```{figure} ../assets/common/setup/qgc-firmware.webp
:alt: Загрузка прошивки в QGroundControl
:width: 90%
:align: center

Загрузка прошивки в QGroundControl
```
<br>

6. Откройте *Advanced settings*.
7. В выпадающем меню выберите *Custom firmware file...*.
8. Нажмите *OK* и выберите скачанный файл прошивки.

Дождитесь, пока QGroundControl загрузит прошивку и перезагрузит полетный контроллер.

```{warning}
Не отключайте полетный контроллер от компьютера во время загрузки прошивки.
```

## Настройка полетного контроллера

```{figure} ../assets/common/setup/qgc-requires-setup.webp
:alt: Обзор настроек QGroundControl
:width: 90%
:align: center

Обзор настроек QGroundControl
```
<br>

После загрузки прошивки проверьте основные пункты настройки:

1. *Airframe* - выбор рамы.
2. *Radio* - настройка аппаратуры управления.
3. *Sensors* - калибровка датчиков.
4. *Flight Modes* - настройка полетных режимов.

### Выбор рамы

```{figure} ../assets/common/setup/qgc-frame-apply.webp
:alt: Выбор рамы в QGroundControl
:width: 90%
:align: center

Выбор рамы в QGroundControl
```
<br>

1. Выберите меню *Airframe*.
2. Выберите тип рамы *Quadrotor*.
3. Выберите подтип рамы *Generic Quadrotor*.
4. Переместитесь в начало списка и нажмите *Apply and Restart*.
5. Подтвердите действие кнопкой *Apply*.
6. Дождитесь применения настроек и перезагрузки полетного контроллера.

### Параметры

Для настройки параметров полетного контроллера перейдите в *Vehicle Configuration* и выберите меню *Parameters*. Используйте поле *Search*, чтобы быстро найти параметр по имени.

```{figure} ../assets/common/setup/qgc-parameters.webp
:alt: Параметры QGroundControl
:width: 90%
:align: center

Параметры QGroundControl
```
<br>

После изменения параметра нажмите *Save*. Если QGroundControl попросит перезагрузить контроллер, нажмите *Tools*, затем *Reboot vehicle*.

### Файлы с параметрами

Готовый набор параметров можно загрузить через меню *Parameters*. Для этого нажмите *Tools*, выберите *Load from file...* и укажите файл `.params`.

- Параметры для Clover 5: [clover5.params](@assets@/downloads/clover5.params)

## Усредненные PID-коэффициенты

```{note}
PID-коэффициенты зависят от конкретной сборки квадрокоптера. Значения ниже подходят как отправная точка, но для точного полета параметры нужно подбирать вручную.
```

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

**Далее**: [Калибровка датчиков](Setup/calibration.md).
