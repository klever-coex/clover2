# Прошивка

В этой статье описана замена прошивки PX4 на Betaflight на полетном контроллере **MicoAir743v2** с помощью режима DFU.

```{caution}
Инструкция подходит только для полeтного контроллера **MicoAir743v2**. Полная очистка памяти удалит установленную прошивку PX4 и ее настройки. Перед началом работы убедитесь, что подключили нужный контроллер и скачали правильный файл прошивки.
```

```{figure} @assets@/ru/assembly/clover5-fpv/firmware/micoair743v2.webp
:alt: Полётный контроллер MicoAir743v2
:width: 350px
:align: center
Полетный контроллер MicoAir743v2
```

<br>

## Необходимые программы и файлы

- [Betaflight App](https://app.betaflight.com/) — приложение для сборки прошивки и настройки полетного контроллера;
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) — программа для очистки памяти контроллера и записи прошивки;
- файл прошивки Betaflight для **MicoAir743v2** в формате `.hex` — соберите и сохраните его по инструкции ниже.

До начала прошивки скачайте необходимые файлы и установите **STM32CubeProgrammer**.

## Сборка и сохранение файла прошивки

1. Откройте [Betaflight App](https://app.betaflight.com/) и перейдите во вкладку `Программатор` *(Firmware Flasher)*.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/build-open-flasher.webp
    :alt: Открытие программатора Betaflight
    :width: 700px
    :align: center
    Открытие программатора Betaflight
    ```

    <br>

2. На вкладке `Плата и сборка` *(Board & Build)* выберите плату `MICOAIR743V2` и нужную версию прошивки.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/build-board-selection.webp
    :alt: Выбор платы и релиза прошивки
    :width: 700px
    :align: center
    Выбор платы и версии прошивки
    ```

    <br>

3. В поле `Протокол OSD` *(OSD Protocol)* выберите `Analog`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/build-osd-protocol.webp
    :alt: Выбор протокола OSD Analog
    :width: 700px
    :align: center
    Выбор протокола OSD Analog
    ```

    <br>

4. Нажмите `Загрузить прошивку [Online]` *(Load Firmware [Online])* и дождитесь завершения сборки прошивки.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/build-load-firmware.webp
    :alt: Запуск сборки прошивки
    :width: 700px
    :align: center
    Запуск сборки прошивки
    ```

    <br>

5. Перейдите на вкладку `Прошивка` *(Flash)*. Нажмите `Save`, чтобы сохранить собранную прошивку в формате `HEX` на компьютер. Этот файл потребуется в STM32CubeProgrammer.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/build-save-hex.webp
    :alt: Сохранение собранного HEX-файла
    :width: 700px
    :align: center
    Сохранение собранного HEX-файла
    ```

    <br>

(clover5-fpv-dfu-mode)=

## Вход в режим DFU

1. Отключите полетный контроллер от компьютера.
2. Зажмите кнопку `BOOT` на контроллере.
3. Не отпуская кнопку `BOOT`, подключите контроллер к компьютеру по USB.
4. Убедитесь, что компьютер распознал контроллер в режиме DFU. В **STM32CubeProgrammer** устройство может отображаться как DFU-устройство или STM32 Bootloader.

## Установка Betaflight

1. Откройте **STM32CubeProgrammer** и нажмите кнопку выбора интерфейса подключения.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/select-connection-interface.webp
    :alt: Выбор интерфейса подключения в STM32CubeProgrammer
    :width: 700px
    :align: center
    Выбор интерфейса подключения в STM32CubeProgrammer
    ```

    <br>

2. Выберите интерфейс `USB`.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/select-usb-interface.webp
    :alt: Выбор интерфейса USB
    :width: 700px
    :align: center
    Выбор интерфейса USB
    ```

    <br>

3. Нажмите кнопку обновления справа от списка `Port`. **STM32CubeProgrammer** должен обнаружить контроллер, подключенный в режиме DFU. Если устройство не появилось в списке, повторите действия из раздела {ref}`Вход в режим DFU <clover5-fpv-dfu-mode>`.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/refresh-dfu-device.webp
    :alt: Поиск DFU-устройства
    :width: 700px
    :align: center
    Поиск DFU-устройства
    ```

    <br>

4. Нажмите `Connect`, чтобы подключиться к контроллеру.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/connect-dfu-device.webp
    :alt: Подключение к контроллеру в режиме DFU
    :width: 700px
    :align: center
    Подключение к контроллеру в режиме DFU
    ```

    <br>

5. Выполните полную очистку памяти: нажмите кнопку с изображением ластика в левом нижнем углу окна, подтвердите действие во всплывающем окне и дождитесь завершения операции.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/full-chip-erase.webp
    :alt: Полная очистка памяти контроллера
    :width: 700px
    :align: center
    Полная очистка памяти контроллера
    ```

    <br>

6. Нажмите `Open file` и выберите скачанный файл прошивки `.hex` для **MicoAir743v2**. Имя файла зависит от версии прошивки.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/open-firmware-file.webp
    :alt: Выбор файла прошивки Betaflight
    :width: 700px
    :align: center
    ```

    <br>

7. Нажмите `Download` и дождитесь завершения записи прошивки.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/download-firmware.webp
    :alt: Загрузка прошивки Betaflight
    :width: 700px
    :align: center
    Загрузка прошивки Betaflight
    ```

    <br>

8. После успешной записи нажмите `Disconnect`.

    ```{figure} @assets@/common/assembly/clover5-fpv/firmware/disconnect-programmer.webp
    :alt: Отключение контроллера от STM32CubeProgrammer
    :width: 700px
    :align: center
    Отключение контроллера от STM32CubeProgrammer
    ```

    <br>

9. Отключите контроллер от USB, подключите его снова, не нажимая кнопку `BOOT` и откройте **Betaflight App**.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/betaflight-configurator.webp
    :alt: Полётный контроллер в Betaflight Configurator
    :width: 700px
    :align: center
    Полетный контроллер в Betaflight Configurator
    ```

    <br>

После установки прошивки перейдите к {doc}`настройке квадрокоптера <BetaflightSetup>`.
