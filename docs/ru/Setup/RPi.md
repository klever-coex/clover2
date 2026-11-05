# Raspberry Pi

```{toctree}
:titlesonly:
:maxdepth: 1
:hidden:

RPi/DroneConnection
RPi/ArucoMap
RPi/DroneSettings
```

Для записи образа на карту памяти Raspberry Pi используйте программу **balenaEtcher**.

## Подготовка

1. Скачайте **balenaEtcher** с [официального сайта](https://etcher.balena.io/), установите и запустите программу на компьютере.
2. Скачайте [актуальный образ для Raspberry Pi](https://klever-doc.tech/ru/p/klever5).
3. Подключите карту памяти microSD к компьютеру с помощью картридера.

```{warning}
При записи образа все данные на выбранном накопителе будут удалены. Сохраните нужные файлы с карты памяти и убедитесь, что выбрали правильный накопитель.
```

## Запись образа на карту памяти

1. В главном окне **balenaEtcher** нажмите `Flash from file`.

    ```{figure} @assets@/common/setup/rpi/etcher-main-screen.webp
    :alt: balenaEtcher
    :width: 90%
    :align: center

    balenaEtcher
    ```

    <br>

2. Выберите скачанный файл образа. Архив распаковывать не обязательно. После выбора в окне появятся имя и размер образа.

    ```{figure} @assets@/common/setup/rpi/etcher-image-selected.webp
    :alt: Выбранный файл образа в balenaEtcher
    :width: 90%
    :align: center

    Выбранный файл образа
    ```

    <br>

3. Нажмите `Select target`. В списке накопителей отметьте карту памяти и нажмите `Select 1`. Проверьте название и объем выбранного накопителя.

    ```{figure} @assets@/common/setup/rpi/etcher-select-target.webp
    :alt: Выбор карты памяти в списке накопителей balenaEtcher
    :width: 90%
    :align: center

    Выбор карты памяти
    ```

    <br>

4. Проверьте выбранные образ и карту памяти. Нажмите `Flash!`. Если система запросит права администратора, подтвердите действие.

    ```{figure} @assets@/common/setup/rpi/etcher-ready-to-flash.webp
    :alt: Выбранные образ и карта памяти перед началом записи
    :width: 90%
    :align: center

    Запуск записи образа
    ```

    <br>

5. Дождитесь завершения записи и проверки данных. Не извлекайте карту памяти и не отключайте картридер во время записи и проверки.

    ```{figure} @assets@/common/setup/rpi/etcher-flashing.webp
    :alt: Процесс записи образа
    :width: 90%
    :align: center

    Запись образа
    ```

    <br>

6. После успешного завершения появится сообщение `Flash Completed!`.

    ```{figure} @assets@/common/setup/rpi/etcher-flash-completed.webp
    :alt: Сообщение Flash Completed! об успешной записи образа
    :width: 90%
    :align: center

    Завершение записи образа
    ```

    <br>

7. Безопасно извлеките карту памяти из компьютера и установите ее в Raspberry Pi.

