# Прошивка

```{tip}
В этой статье описана замена прошивки PX4 на Betaflight на полётном контроллере **MicoAir743v2** через режим DFU.
```

```{caution}
Инструкция подходит только для полётного контроллера **MicoAir743v2**. Полная очистка памяти удалит установленную прошивку PX4 и её настройки. Перед началом убедитесь, что выбран правильный контроллер и файл прошивки.
```

```{figure} @assets@/ru/assembly/clover5-fpv/firmware/micoair743v2.webp
:alt: Полётный контроллер MicoAir743v2
:width: 350px
:align: center
```

## Содержание

```{contents}
:local:
:depth: 1
```

## Необходимые программы и файлы

- [Betaflight Configurator](https://github.com/betaflight/betaflight-configurator/releases) — официальный раздел релизов;
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) — программа для очистки памяти и загрузки прошивки;
- [прошивка Betaflight для MicoAir743v2](https://drive.google.com/file/d/1gaLgiCqa-gUa-vJtmGSQhVh7zwCFcqFN/view?usp=drive_link) — файл `MicoAir743v2_Betaflight-4.5.2.hex`.

Установите STM32CubeProgrammer до начала прошивки.

(clover5-fpv-dfu-mode)=

## Вход в режим DFU

1. Отключите полётный контроллер от компьютера.
2. Зажмите кнопку **BOOT** на контроллере.
3. Не отпуская кнопку **BOOT**, подключите контроллер к компьютеру по USB.
4. Убедитесь, что устройство определилось в системе как DFU-устройство или STM32 Bootloader.

## Установка Betaflight

1. Откройте STM32CubeProgrammer и нажмите кнопку выбора интерфейса подключения, на которой по умолчанию указан **UART**.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/select-connection-interface.webp
    :alt: Выбор интерфейса подключения в STM32CubeProgrammer
    :width: 700px
    :align: center
    ```

2. Выберите интерфейс **USB**.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/select-usb-interface.webp
    :alt: Выбор интерфейса USB
    :width: 700px
    :align: center
    ```

3. Нажмите кнопку обновления справа от списка **Port**. STM32CubeProgrammer должен автоматически выбрать DFU-устройство. Если устройство не появилось, повторите действия из раздела {ref}`Вход в режим DFU <clover5-fpv-dfu-mode>`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/refresh-dfu-device.webp
    :alt: Поиск DFU-устройства
    :width: 700px
    :align: center
    ```

4. Нажмите **Connect**.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/connect-dfu-device.webp
    :alt: Подключение к контроллеру в режиме DFU
    :width: 700px
    :align: center
    ```

5. Выполните полную очистку памяти: нажмите кнопку с изображением ластика в левом нижнем углу, подтвердите действие во всплывающем окне и дождитесь завершения операции.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/full-chip-erase.webp
    :alt: Полная очистка памяти контроллера
    :width: 700px
    :align: center
    ```

6. Нажмите **Open file** и выберите файл `MicoAir743v2_Betaflight-4.5.2.hex`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/open-firmware-file.webp
    :alt: Выбор файла прошивки Betaflight
    :width: 700px
    :align: center
    ```

7. Нажмите **Download** и дождитесь окончания загрузки прошивки.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/download-firmware.webp
    :alt: Загрузка прошивки Betaflight
    :width: 700px
    :align: center
    ```

8. После успешной загрузки нажмите **Disconnect**.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/disconnect-programmer.webp
    :alt: Отключение контроллера от STM32CubeProgrammer
    :width: 700px
    :align: center
    ```

9. Отключите контроллер от USB, подключите его снова без нажатия кнопки **BOOT** и откройте Betaflight Configurator.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware/betaflight-configurator.webp
    :alt: Полётный контроллер в Betaflight Configurator
    :width: 700px
    :align: center
    ```

После установки прошивки перейдите к {doc}`настройке квадрокоптера <BetaflightSetup>`.
