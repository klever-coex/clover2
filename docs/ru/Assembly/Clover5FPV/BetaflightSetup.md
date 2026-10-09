# Настройка


В этой статье описана базовая настройка квадрокоптера **«Клевер 5 ФПВ»** с полетным контроллером **MicoAir743v2** в **Betaflight Configurator**. 

```{tip}
Перед началом настройки установите прошивку Betaflight на контроллер.
```

```{caution}
Перед настройкой и проверкой электродвигателей обязательно снимите пропеллеры.
```

```{contents}
:local:
:depth: 1
```

## Подключение и подготовка

1. Подключите полетный контроллер к компьютеру по USB и откройте **Betaflight Configurator**.
2. Во вкладке `Программатор` *(Firmware Flasher)* включите режим эксперта *(Enable Expert Mode)*.

3. Нажмите `Подключиться` *(Connect)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/configurator-connection.webp
    :alt: Подключение полётного контроллера в Betaflight Configurator
    :width: 700px
    :align: center
    ```

## Калибровка и базовая конфигурация

1. Если появилось предупреждение `Акселерометр не откалиброван`, выполните калибровку. Перейдите во вкладку `Датчики` *(Sensors)*. Нажмите `Калибровать` *(Calibrate)*.
2. Отключите ненужные датчики, если они не используются.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/accelerometer-calibration.webp
    :alt: Калибровка акселерометра
    :width: 700px
    :align: center
    Калибровка акселерометра
    ```

     ```{figure} @assets@/ru/assembly/clover5-fpv/setup/basic-configuration.webp
    :alt: Базовая конфигурация квадрокоптера
    :width: 700px
    :align: center
    Базовая конфигурация квадрокоптера
    ```

3. Перейдите во вкладку `Конфигурация` *(Configuration)*. В разделе `Персонализация` *(Personalization)*:
* В поле `Название борта` *(Craft name)* укажите имя квадрокоптера.
* В поле `Имя пилота` *(Pilot name)* при необходимости укажите имя пилота.

4. В разделе `Прочий функционал` *(Other features)* включите включите `AIRMODE`, `DISPLAY`, `LED_STRIP`, `OSD`.
.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/feature-configuration.webp
    :alt: Включение дополнительных функций Betaflight
    :width: 700px
    :align: center
    Включение дополнительных функций Betaflight
    ```

5. Нажмите `Сохранить и Перезагрузить` *(Save and Reboot)*.

## Настройка портов

1. Откройте вкладку `Порты` *(Ports)*.
2. Настройте порты следующим образом:

   - **UART2**: выключите `MSP` и в разделе `Периферия` *(Peripherals)* выберите `VTX` *(IRC Tramp)*;
   - **UART3**: если подключён GPS, в разделе `Вход датчиков` *(Sensor Input)* выберите `GPS` и скорость `57600`;
   - **UART6**: включите `Serial RX` для приемника;
   - **UART7**: если используется телеметрия ESC, выберите `ESC`.

3. Нажмите `Сохранить и Перезагрузить` *(Save and Reboot)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/port-configuration.webp
    :alt: Настройка портов полётного контроллера
    :width: 700px
    :align: center
    Настройка портов полетного контроллера
    ```

```{caution}
Если приемник, GPS или VTX подключены к другим UART, выставляйте настройки в соответствии с фактическим подключением.
```

## Настройка приемника

1. Откройте вкладку `Приемник` *(Receiver)*.
2. В поле `Режимы приемника` *(Receiver Mode)* выберите `Серийный порт (через UART)` *(Serial (via UART))*.
3. В  поле `Приемник с последовательным портом` *(Serial Receiver Provider)* выберите `CRSF`.
4. Проверьте движение стиков в области предварительного просмотра.
5. Во вкладке `Карта каналов` *(Channel map)* используйте `AETR1234`.
6. При необходимости выполните привязку приемника кнопкой `Привязать приемник` *(Bind Receiver)*.
7. Нажмите `Сохранить` *(Save)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/receiver-configuration.webp
    :alt: Настройка приёмника CRSF
    :width: 700px
    :align: center
    Настройка приемника CRSF
    ```

## Настройка режимов

1. Откройте вкладку `Режимы` *(Modes)*.
2. Установите значения:

   - `ARM` на `AUX1` в диапазоне `1700–2100`;
   - `ANGLE` на `AUX3` в диапазоне `1300–1700`;
   - `HORIZON` на `AUX3` в диапазоне `1700–2100`;
   - `FAILSAFE` на `AUX4` в диапазоне `1700–2100`;
   - `BEEPER` на `*AUX4` в диапазоне `1700–2100`.

3. Нажмите `Сохранить` *(Save)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/arm-mode.webp
    :alt: Настройка режима ARM
    :width: 700px
    :align: center
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/angle-mode.webp
    :alt: Настройка режима ANGLE
    :width: 700px
    :align: center
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/horizon-mode.webp
    :alt: Настройка режима HORIZON
    :width: 700px
    :align: center
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/failsafe-beeper-modes.webp
    :alt: Настройка режимов FAILSAFE и BEEPER
    :width: 700px
    :align: center
    ```

## Настройка электродвигателей

```{caution}
Перед настройкой и проверкой электродвигателей снимите пропеллеры.
```

1. Откройте вкладку `Моторы` *(Motors)*.
2. В поле `Микшер` *(Mixer)* выберите `QUAD X`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/quad-x-mixer.webp
    :alt: Выбор схемы QUAD X
    :width: 700px
    :align: center
    Выбор схемы QUAD X
    ```

3. В разделе `Настройки ESC / Моторов` *(ESC/Motor Features)* выставьте:

   - `Протокол ESC / Мотора` *(ESC/Motor protocol)*: `DSHOT600`;
   - `ESC_SENSOR`: *включено*;
   - `Двусторонний DShot` *(Bidirectional DShot)*: *включено*;
   - `Полюса мотора` *(Motor poles)*: `14`;
   - `Холостой ход (%)` *(Motor Idle (% static))*: `5.5`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/motor-features.webp
    :alt: Настройка ESC и моторов
    :width: 700px
    :align: center
    Настройка ESC и моторов
    ```

4. Включите `Обратное вращение моторов` *(Motor direction is reversed)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/reversed-motor-direction.webp
    :alt: Выбор обратного направления вращения моторов
    :width: 700px
    :align: center
    Выбор обратного направления вращения моторов
    ```

5. Для настройки направления вращения нажмите `Направление мотора` *(Motor direction)*.
6. Подтвердите предупреждение о снятых пропеллерах.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/motor-direction-safety-warning.webp
    :alt: Предупреждение перед настройкой направления моторов
    :width: 700px
    :align: center
    Предупреждение перед настройкой направления моторов
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/motor-direction-wizard.webp
    :alt: Мастер настройки направления моторов
    :width: 700px
    :align: center
    Мастер настройки направления моторов
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/motor-direction-method.webp
    :alt: Выбор способа настройки направления моторов
    :width: 700px
    :align: center
    Выбор способа настройки направления моторов
    ```

7. Для настройки порядка электродвигателей нажмите `Переназначение моторов` *(Reorder motors)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/motor-reordering-start.webp
    :alt: Запуск настройки порядка моторов
    :width: 700px
    :align: center
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/motor-reordering.webp
    :alt: Настройка порядка моторов
    :width: 700px
    :align: center
    ```

8. Если электродвигатели вращаются не в ту сторону, измените направление через `Мастер настройки` *(Wizard)*.

## Настройка OSD

1. Откройте вкладку `OSD`.
2. Для аналоговой видеосистемы выберите формат видеосигнала `Авто` *(Auto)*.
3. В разделе `Единицы измерения` *(Units)* выберите `Метрические` *(Metric)*.
4. Настройте таймеры:

   - **Таймер 1 (Timer 1)**: `Время работы (On time)`;
   - **Таймер 2 (Timer 2)**: `Полный период работы (Total armed time)`.

5. В разделе `Сигналы` *(Alarms)* установите следующие значения:

   - `RSSI`: `20`;
   - `Емкость` *(Capacity)*: `2200`;
   - `Высота` *(Altitude)*: `100`;
   - `Качество соединения` *(Link Quality)*: `60`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/osd-settings.webp
    :alt: Настройка параметров OSD
    :width: 700px
    :align: center
    Настройка параметров OSD
    ```

6. Включите и разместите на экране основные элементы OSD:

   - `Название борта` *(Craft name)*;
   - `Напряжение батареи` *(Battery voltage)*;
   - `Disarmed`;
   - `Режим полета` *(Fly mode)*;
   - `Широта GPS` *(GPS latitude)*;
   - `Долгота GPS` *(GPS longitude)*;
   - `Положение газа` *(Throttle position)*;
   - `Таймер 1` *(Timer 1)*;
   - `Таймер 2` *(Timer 2)*;
   - `Канал VTX` *(VTX channel)*;
   - `Предупреждения` *(Warnings)*.

7. После размещения элементов нажмите `Сохранить` *(Save)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/osd-layout.webp
    :alt: Размещение элементов OSD
    :width: 700px
    :align: center
    Размещение элементов OSD
    ```

## Настройка системы видеопередачи

```{caution}
Перед подачей питания подключите антенну к видеопередатчику. Не включайте видеопередатчик без антенны, так как это может привести к его повреждению. Используйте только разрешенные в вашем регионе частоты и мощность передатчика.
```

1. Откройте вкладку `Видеопередатчик` *(Video Transmitter)*.
2. Если таблица VTX ещё не загружена, нажмите `Загрузить из файла` *(Load from file)*.
3. Выберите файл VTX-таблицы.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/vtx-settings.webp
    :alt: Настройка видеопередатчика
    :width: 700px
    :align: center
    Настройка видеопередатчика
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/vtx-table.webp
    :alt: Таблица частот и мощности видеопередатчика
    :width: 700px
    :align: center
    ```

4. После загрузки таблицы проверьте параметры:

   - `Сетка` *(Band)*: `RACEBAND`;
   - `Канал` *(Channel)*: `Канал 5` *(Channel 5)*;
   - `Мощность` *(Power)*: `25`;
   - `Режим пит-стопа` *(Pit Mode)*: *по необходимости*;
   - `Низкая мощность при дизарме` *(Low Power Disarm)*: `Выкл (Off)`.

5. Убедитесь, что таблица частот и уровней мощности загрузилась корректно.
6. Нажмите `Сохранить` *(Save)*.

## Настройка LED-ленты

1. Убедитесь, что функция `Поддержка разноцветной LED ленты` *(LED_STRIP)* уже включена во вкладке `Конфигурация` *(Configuration)*.
2. Откройте вкладку `LED лента` *(LED Strip)*.
3. Разметьте используемые светодиоды на сетке.
4. Нажмите `Режим назначения цепи` *(Wire Ordering Mode)* и задайте порядок светодиодов.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/led-strip-layout.webp
    :alt: Разметка светодиодов на сетке
    :width: 700px
    :align: center
    Разметка светодиодов на сетке
    ```

5. В поле `Функция` *(Function)* выберите `Цвет` *(Color)*.
6. При необходимости в разделе `Изменение цвета` *(Color modifier)* выберите режим, например `Газ` *(Throttle)*.
7. Назначьте цвета по своему усмотрению и нажмите `Сохранить` *(Save)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/led-strip-wire-order.webp
    :alt: Настройка порядка светодиодов
    :width: 700px
    :align: center
    Настройка порядка светодиодов
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/led-strip-color-settings.webp
    :alt: Настройка цвета LED-ленты
    :width: 700px
    :align: center
    Настройка цвета LED-ленты
    ```

    ```{figure} @assets@/ru/assembly/clover5-fpv/setup/led-strip-preview.webp
    :alt: Предварительный просмотр LED-ленты
    :width: 700px
    :align: center
    Предварительный просмотр LED-ленты
    ```

## Заключительная настройка

1. Проверьте, что во вкладке `Конфигурация` *(Configuration)* модель квадрокоптера повторяет реальные движения рамы.
2. Убедитесь, что стики и переключатели корректно отображаются во вкладке `Приемник` *(Receiver)*.
3. Проверьте, что при переключении режимов во вкладке `Режимы` *(Modes)* активируются нужные диапазоны.
4. Проверьте направление вращения электродвигателей без пропеллеров.
5. Убедитесь, что OSD отображается в FPV-шлеме.

Устанавливайте пропеллеры только после завершения всех проверок перед первым тестовым запуском.
