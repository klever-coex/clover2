# Настройка

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

## Установка QGroundControl

Используйте программное обеспечение QGroundControl для прошивки, настройки и калибровки полетного контроллера.

Скачайте установочный файл для Windows, Linux или macOS на странице [релиза QGroundControl v5.1.4](https://github.com/mavlink/QGroundControl/releases/tag/v5.1.4) и установите на ваш компьютер. Если установщик предложит поставить дополнительные драйверы, согласитесь с установкой.

Дополнительная документация доступна на сайте [QGroundControl User Guide](https://docs.qgroundcontrol.com/).

## Загрузка прошивки в полетный контроллер

Скачайте актуальную стабильную версию прошивки.

- Прошивка PX4 1.16.1 для MicoAir H743 V2: [micoair_h743-v2_default.px4](@assets@/downloads/micoair_h743-v2_default.px4)

Для загрузки прошивки:

1. Отключите полетный контроллер от компьютера, если он подключен.
2. Запустите QGroundControl.
3. Перейдите в панель *Vehicle Configuration*.
4. Выберите меню *Firmware*.
5. Подключите полетный контроллер к компьютеру с помощью USB-кабеля.
6. В меню выберите последовательный порт полетного контроллера.
7. QGroundControl предложит перевести контроллер в режим загрузчика, отключите USB-кабель и подключите его снова.

```{figure} ../assets/common/setup/qgc-port-apply.webp
:alt: Переподключение полётного контроллера для загрузки прошивки
:width: 90%
:align: center

Переподключение полетного контроллера для загрузки прошивки
```
<br>

8. Дождитесь, когда QGroundControl обнаружит полетный контроллер. В появившемся окне выберите *PX4 Flight Stack*.
9. Откройте *Advanced settings*.
10. В выпадающем меню выберите *Custom firmware file...*. Нажмите *OK*.

```{figure} ../assets/common/setup/qgc-firmware.webp
:alt: Загрузка прошивки в QGroundControl
:width: 90%
:align: center

Загрузка прошивки в QGroundControl
```
<br>

11. Выберите ранее скачанный файл прошивки.

```{warning}
Не отключайте полетный контроллер от компьютера во время загрузки прошивки.
```

Дождитесь завершения загрузки прошивки и автоматической перезагрузки полетного контроллера.

## Настройка полетного контроллера

```{figure} ../assets/common/setup/qgc-requires-setup.webp
:alt: Обзор настроек QGroundControl
:width: 90%
:align: center

Обзор настроек QGroundControl
```
<br>

После установки прошивки выполните настройку следующих параметров:

1. *Airframe* — конфигурация рамы.
2. *Radio* — аппаратура управления.
3. *Sensors* — датчики.
4. *Flight Modes* — полетные режимы.

### Настройка конфигурации рамы

```{figure} ../assets/common/setup/qgc-frame-apply.webp
:alt: Выбор рамы в QGroundControl
:width: 90%
:align: center

Окно выбора конфигурации рамы в QGroundControl
```
<br>

1. Перейдите в панель *Vehicle Configuration*.
2. Выберите меню *Airframe*.
3. Выберите тип рамы *Quadrotor X*.
4. Выберите подтип рамы *Generic Quadcopter*.
5. Переместитесь в начало списка и нажмите *Apply and Restart*.
6. Подтвердите применение настроек нажатием *Apply*.
7. Дождитесь завершения настройки и перезагрузки полетного контроллера.

### Настройка параметров

1. Перейдите в панель *Vehicle Configuration*.
2. Выберите меню *Parameters*. Для поиска параметров используйте поле *Search*.
3. Установите требуемые значения параметров. После изменения каждого параметра нажмите *Save*.

```{figure} ../assets/common/setup/qgc-parameters.webp
:alt: Параметры QGroundControl
:width: 90%
:align: center

Параметры QGroundControl
```
<br>

При необходимости перезагрузите полетный контроллер через меню *Tools*, затем *Reboot vehicle*.

Вы можете настроить основные параметры, загрузив файл параметров:

1. Перейдите в меню *Parameters*.
2. Нажмите *Tools*.
3. Выберите *Load from file...*.
4. Выберите соответствующий файл с расширением `.params`.
5. Дождитесь завершения загрузки параметров.
6. После загрузки проверьте примененные значения параметров.

- Параметры для Clover 5: [clover5.params](@assets@/downloads/clover5.params)

## Настройка ПИД-регулятора

Используйте следующие усредненные коэффициенты ПИД-регуляторов.

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
Необходимо учитывать, что для идеального полета параметры ПИД-регуляторов подбираются вручную для каждого конкретного собранного квадрокоптера.
```
