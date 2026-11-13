# Возврат на PX4

В этой статье описана замена прошивки Betaflight на PX4 на полетном контроллере **MicoAir743v2** с помощью режима DFU.


```{caution}
Инструкция подходит только для полетного контроллера **MicoAir743v2**. Полная очистка памяти удалит Betaflight и все ее настройки.
```

```{figure} @assets@/ru/assembly/clover5-fpv/firmware/micoair743v2.webp
:alt: Полетный контроллер MicoAir743v2
:width: 350px
:align: center

Полетный контроллер MicoAir743v2
```

<br>

## Необходимые программы и файлы

- [QGroundControl](https://github.com/mavlink/QGroundControl/releases) — программа для установки прошивки PX4 и настройки полетного контроллера;
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) — программа для очистки памяти контроллера и записи загрузчика;
- [прошивка PX4 для MicoAir743v2](@assets@/downloads/micoair_h743-v2_default.px4) в формате `.px4` — установите через QGroundControl;
- [загрузчик PX4 для MicoAir743v2](@assets@/downloads/micoair_h743-v2_bootloader.bin) в формате `.bin` — запишите через STM32CubeProgrammer перед установкой прошивки.

До начала прошивки скачайте необходимые файлы и установите программы **STM32CubeProgrammer** и **QGroundControl**.

## Запись загрузчика PX4

1. Зажмите кнопку `BOOT` на полетном контроллере. Удерживая кнопку, подключите контроллер к компьютеру по USB.

2. Откройте **STM32CubeProgrammer** и нажмите кнопку выбора интерфейса подключения.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/select-connection-interface.webp
    :alt: Выбор интерфейса подключения в STM32CubeProgrammer
    :width: 700px
    :align: center

    Выбор интерфейса подключения в STM32CubeProgrammer
    ```

    <br>

3. Выберите интерфейс `USB`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/select-usb-interface.webp
    :alt: Выбор интерфейса USB
    :width: 700px
    :align: center

    Выбор интерфейса USB
    ```

    <br>

4. Нажмите кнопку обновления справа от списка `Port`. Если программа не обнаружила порт, повторно подключите контроллер к компьютеру, удерживая кнопку `BOOT`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/refresh-dfu-port.webp
    :alt: Поиск DFU-устройства
    :width: 700px
    :align: center

    Поиск DFU-устройства
    ```

    <br>

5. Нажмите `Connect`, чтобы подключиться к контроллеру.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/connect-dfu-device.webp
    :alt: Подключение к контроллеру в режиме DFU
    :width: 700px
    :align: center

    Подключение к контроллеру в режиме DFU
    ```

    <br>

6. Выполните полную очистку памяти контроллера. Для этого нажмите кнопку с изображением ластика в левом нижнем углу окна, подтвердите действие во всплывающем окне и дождитесь завершения операции.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/erase-flash.webp
    :alt: Полная очистка памяти контроллера
    :width: 700px
    :align: center

    Полная очистка памяти контроллера
    ```

    <br>

7. Нажмите `Open file` и выберите скачанный файл загрузчика `micoair_h743-v2_bootloader.bin`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/open-bootloader-file.webp
    :alt: Выбор файла загрузчика PX4
    :width: 700px
    :align: center

    Выбор файла загрузчика PX4
    ```

    <br>

8. Нажмите `Download` и дождитесь завершения записи загрузчика.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/write-bootloader.webp
    :alt: Запись загрузчика PX4
    :width: 700px
    :align: center

    Запись загрузчика PX4
    ```

    <br>

9. После успешной записи нажмите `Disconnect`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/disconnect-programmer.webp
    :alt: Отключение контроллера от STM32CubeProgrammer
    :width: 700px
    :align: center

    Отключение контроллера от STM32CubeProgrammer
    ```

    <br>

10. Закройте **STM32CubeProgrammer** и отключите контроллер от компьютера.

## Установка прошивки PX4

1. Откройте **QGroundControl**.
2. Подключите полетный контроллер к компьютеру по USB, не нажимая кнопку `BOOT`.
3. Нажмите на логотип **QGroundControl** в левом верхнем углу.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/open-qgc-menu.webp
    :alt: Главное меню QGroundControl
    :width: 700px
    :align: center

    Главное меню QGroundControl
    ```

    <br>

4. В выпадающем меню выберите `Configure`. Откроется панель `Vehicle Configuration`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/open-vehicle-configuration.webp
    :alt: Пункт Configure в выпадающем меню QGroundControl
    :width: 700px
    :align: center

    Пункт Configure в выпадающем меню QGroundControl
    ```

    <br>

5. Выберите раздел `Firmware`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/open-firmware-menu.webp
    :alt: Переход в меню Firmware
    :width: 700px
    :align: center

    Переход в меню Firmware
    ```

    <br>

6. Выберите контроллер в списке портов и нажмите `Flash`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/start-firmware-update.webp
    :alt: Выбор контроллера и запуск прошивки
    :width: 700px
    :align: center

    Выбор контроллера и запуск прошивки
    ```

    <br>

7. Если QGroundControl предложит переподключить контроллер, выполните запрос без нажатия кнопки `BOOT`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/reconnect-controller.webp
    :alt: Запрос на переподключение контроллера
    :width: 700px
    :align: center

    Запрос на переподключение контроллера
    ```

    <br>

8. В окне `Firmware Setup` включите `Advanced settings`, выберите `Custom firmware file...` и нажмите `OK`. Укажите скачанный файл прошивки `micoair_h743-v2_default.px4`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/select-custom-firmware.webp
    :alt: Выбор прошивки из файла
    :width: 700px
    :align: center

    Выбор прошивки из файла
    ```

    <br>

9. Дождитесь завершения записи прошивки.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/wait-for-firmware-upload.webp
    :alt: Запись прошивки PX4
    :width: 700px
    :align: center

    Запись прошивки PX4
    ```

    <br>

10. Перейдите к {doc}`настройке полетного контроллера <../../../Setup/QGC>`.

    ```{figure} @assets@/ru/assembly/clover5-fpv/firmware-px4/configure-flight-controller.webp
    :alt: Полетный контроллер после установки PX4
    :width: 700px
    :align: center

    Полетный контроллер после установки PX4
    ```
