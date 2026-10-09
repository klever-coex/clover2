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

```{contents}
:local:
:depth: 1
```

## Необходимые программы и файлы

- [Betaflight Configurator](https://github.com/betaflight/betaflight-configurator/releases) — программа для настройки полетного контроллера;
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) — программа для очистки памяти контроллера и записи прошивки;
- [прошивка Betaflight для MicoAir743v2](https://drive.google.com/file/d/1gaLgiCqa-gUa-vJtmGSQhVh7zwCFcqFN/view?usp=drive_link) — файл `MicoAir743v2_Betaflight-4.5.2.hex`.

До начала прошивки скачайте необходимые файлы и установите **STM32CubeProgrammer**.

(clover5-fpv-dfu-mode)=

## Вход в режим DFU

1. Отключите полетный контроллер от компьютера.
2. Зажмите кнопку `BOOT` на контроллере.
3. Не отпуская кнопку `BOOT`, подключите контроллер к компьютеру по USB.
4. Убедитесь, что компьютер распознал контроллер в режиме DFU. В **STM32CubeProgrammer** устройство может отображаться как DFU-устройство или STM32 Bootloader.

## Установка Betaflight

1. Откройте **STM32CubeProgrammer** и нажмите кнопку выбора интерфейса подключения. По умолчанию указан `UART`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/select-connection-interface.webp
    :alt: Выбор интерфейса подключения в STM32CubeProgrammer
    :width: 700px
    :align: center
    Выбор интерфейса подключения в STM32CubeProgrammer
    ```

2. Выберите интерфейс `USB`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/select-usb-interface.webp
    :alt: Выбор интерфейса USB
    :width: 700px
    :align: center
    Выбор интерфейса USB
    ```

3. Нажмите кнопку обновления справа от списка `Port`. **STM32CubeProgrammer** должен обнаружить контроллер, подключенный в режиме DFU. Если устройство не появилось в списке, повторите действия из раздела {ref}`Вход в режим DFU <clover5-fpv-dfu-mode>`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/refresh-dfu-device.webp
    :alt: Поиск DFU-устройства
    :width: 700px
    :align: center
    Поиск DFU-устройства
    ```

4. Нажмите `Connect`, чтобы подключиться к контроллеру.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/connect-dfu-device.webp
    :alt: Подключение к контроллеру в режиме DFU
    :width: 700px
    :align: center
    Подключение к контроллеру в режиме DFU
    ```

5. Выполните полную очистку памяти: нажмите кнопку с изображением ластика в левом нижнем углу окна, подтвердите действие во всплывающем окне и дождитесь завершения операции.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/full-chip-erase.webp
    :alt: Полная очистка памяти контроллера
    :width: 700px
    :align: center
    Полная очистка памяти контроллера
    ```

6. Нажмите `Open file` и выберите файл прошивки `MicoAir743v2_Betaflight-4.5.2.hex`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/open-firmware-file.webp
    :alt: Выбор файла прошивки Betaflight
    :width: 700px
    :align: center
    ```

7. Нажмите `Download` и дождитесь завершения записи прошивки.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/download-firmware.webp
    :alt: Загрузка прошивки Betaflight
    :width: 700px
    :align: center
    Загрузка прошивки Betaflight
    ```

8. После успешной записи нажмите `Disconnect`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/disconnect-programmer.webp
    :alt: Отключение контроллера от STM32CubeProgrammer
    :width: 700px
    :align: center
    Отключение контроллера от STM32CubeProgrammer
    ```

9. Отключите контроллер от USB, подключите его снова, не нажимая кнопку `BOOT` и откройте **Betaflight Configurator**.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/betaflight-configurator.webp
    :alt: Полётный контроллер в Betaflight Configurator
    :width: 700px
    :align: center
    Полетный контроллер в Betaflight Configurator
    ```

После установки прошивки перейдите к {doc}`настройке квадрокоптера <BetaflightSetup>`.
